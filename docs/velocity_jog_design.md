# Velocity-Mode Jog — Design Notes

**Status:** Phase-2 design draft, 2026-05-23. Not yet implemented. To be reviewed before any C++ goes in.

## Motivation

The existing jog cycle (`cycle_jogging.cpp`) is a *position-target* operation: caller sends `{"jogx":N}`, firmware ramps from current position to `N` along a fixed exponential velocity profile. It cannot:

- accept mid-flight velocity changes,
- coordinate motion across multiple axes,
- be stopped except by feedhold (`!`) + queue flush (`%`).

This makes it unfit for analog-input use cases — a joystick or pendant wants to say "drive X at +120 mm/min and Y at -80 mm/min, updating as I move the stick, until I tell you to stop." The current FabMo workaround is to stream short G1 segments from the host driver (see `runtime/manual/driver.js`); this works for keypad-style cardinal jogs but stacks the planner queue with several seconds of stale path when the input changes continuously (analog stick sweeps), because feed rate sits ~4× consumption rate (`safetyFactor=4` × `T_RENEW`).

This document specifies a new firmware-level **velocity-mode jog** cycle that takes per-axis target velocities directly, runs them out to the step generator with a shallow queue, and accepts updates on the fly.

## Scope

In-scope:
- New JSON command `{"jgv":...}` to set per-axis target velocities
- A new canonical-machine cycle (`CYCLE_JGV`) that owns motion while jog-velocity is active
- Jerk-limited velocity transitions (use existing `$xjm`)
- Per-axis soft-limit handling (limit hit → zero that axis, others continue)
- Watchdog auto-stop on host silence
- Clean integration with feedhold (`!`), queue flush (`%`), alarm/E-stop, and the existing position-target jog

Out of scope (for now):
- Rotary axes (A/B/C) — straightforward to add later; design supports them but initial implementation may stub
- Pendulum/oscillation modes
- Coordination with arcs or canned cycles

## JSON Protocol

### Set per-axis target velocities

```json
{"jgv":{"x":120, "y":-80, "z":0}}
```

- Values are in **mm/min** (matches `$xvm` velocity-max units; firmware ignores `gun`/units state — caller is responsible for the conversion if running in inches mode).
- Sign encodes direction.
- Magnitude is clamped to `cm->a[axis].velocity_max` per axis.
- **Missing axes hold their previous velocity** (they're not zeroed). This makes it cheap for the host to send partial updates as different sticks/wheels move.
- Sending any nonzero value while not already in `CYCLE_JGV` **enters the cycle**.

### Stop (ramp-down then exit)

```json
{"jgv":{"x":0, "y":0}}    // per-axis, others keep current
{"jgv":{}}                 // shorthand: all axes → 0
```

When all axis targets are 0 and the machine has decelerated to a stop, the cycle exits cleanly (similar to `_jogging_finalize_exit`).

### Hard stop (flush planner, exit immediately)

```json
{"jgv":null}
{"jgv":0}
```

Triggers an internal feedhold + queue flush. Position is preserved (no re-home needed). Use this when the host wants to abort the cycle without waiting for a ramp-down — equivalent to the operator hitting feedhold.

### Status replies

Each `{"jgv":...}` write echoes the *applied* (post-clamp) per-axis velocities back to the caller:

```json
{"r":{"jgv":{"x":120, "y":-80, "z":0, "a":0, "b":0, "c":0}}, ...footer}
```

When the cycle exits, firmware emits `{"jgv":0}` as a one-shot notification — mirrors the existing OMC jog convention.

## Cycle state machine

States (in `cm->cycle_type`):

- `CYCLE_OFF` → `CYCLE_JGV`: triggered by first `{"jgv":...}` with any nonzero target. Saves gcode model state (coord system, distance mode, feedrate mode) like the existing jog cycle.
- `CYCLE_JGV` → `CYCLE_OFF`: triggered by any of:
  - All axis targets are 0 **and** runtime is no longer busy (ramp completed)
  - Watchdog expiry (see below)
  - Internal hard-stop request (`{"jgv":null}` / `{"jgv":0}`)
  - Alarm / shutdown / panic
  - Soft-limit on all active axes simultaneously
- Feedhold (`!`) **pauses** the cycle (does not exit). On resume, motion continues at the still-current target velocities. This matches user expectation: "I tapped pause, now I'm continuing my jog."
- Queue flush (`%`) **exits** the cycle (same as hard-stop).

## Per-segment math

Each cycle-callback invocation, while planner has space and machine is not held:

1. Read current per-axis targets: `v_target[axis]` (mm/min)
2. Compute jerk-limited per-axis end-of-segment velocity:
   `v_end[axis] = v_current[axis] + sign(v_target − v_current) × min(|Δv|, jerk_limit × segment_time)`
   where `jerk_limit` is derived from `$xjm` so a step from 0 to `velocity_max` takes the same time as a normal acceleration phase.
3. Per-axis displacement: `d[axis] = (v_current[axis] + v_end[axis]) / 2 × segment_time / 60` (mm)
4. Toolpath F: `F = ||v_end||` (used by `cm_set_feed_rate_mm`)
5. Soft-limit pre-check: if `pos[axis] + d[axis]` crosses the envelope, set `v_target[axis] = 0` for that axis and recompute (one iteration). Other axes unaffected.
6. Queue via `cm_straight_feed_mm({d[0]…d[N]}, flags, PROFILE_FAST)` — same call the existing jog cycle uses.
7. Advance `v_current ← v_end`.

**Segment time:** target ~5 ms. Short enough that planner queue depth at consumption rate stays ≤ 2 segments (~10 ms of buffered motion). Bounded below by `MIN_SEGMENT_TIME` (g2core constant — to be confirmed during implementation; probably ~1 ms).

**Planner-depth gate:** mirror the existing jog cycle's pattern — only queue a new segment when `mp_planner_is_full(mp)` is false. Otherwise yield.

## Why "hybrid" (planner-queued segments) instead of true planner bypass

Two options were considered for the firmware-internal motion path:

| | **Planner-queued (chosen)** | **True bypass (`st_prep_line` direct)** |
|---|---|---|
| Code touched | New cycle file + small JSON wiring | Modifies `mp_exec_move()` exec dispatch or hooks `kn->idle_task()` |
| Inherits planner safety | Yes (soft-limit, accel, feedhold) | Has to re-implement each |
| Queue depth lag | ~10 ms (2× 5 ms segments) | ~1 ms (single DDA segment in flight) |
| Risk | Low — reuses well-tested path | Higher — direct manipulation of step-gen handshake |

The planner-queued approach gets us from "seconds of lag" to "tens of milliseconds" — within human-perception threshold for joystick control. If field testing shows we need to push below that (drawing curves with the stick, etc.), the planner-bypass path stays open as a follow-up.

## Watchdog

Each `{"jgv":...}` write resets `cm->jogv_last_msg_time`. The cycle callback checks `now - last_msg_time` each iteration. Default timeout: **500 ms**. On expiry:

- All `v_target[axis] = 0`
- Cycle continues running (ramps to stop) — does not hard-exit, so the machine decelerates safely
- A `{"jgv":{"er":"timeout"}}` notification is sent

Rationale: if the host process dies, the USB stack hangs, or the cable comes loose mid-jog, we don't want the machine to keep moving at full speed until it hits a hard limit. 500 ms is short enough to be safe, long enough that normal control gaps (slow host, brief stalls) don't trigger spurious stops.

## Soft-limit interaction

Per-axis: when the planned end-of-segment position would cross the envelope min/max for any axis with that target velocity sign, set `v_target[axis]` = 0 for the **upcoming** segment only. Subsequent `{"jgv":...}` writes can re-enable motion on that axis (in the opposite direction). This matches what the user experiences from the keypad today: pushing into a limit just stops that axis, and other axes keep moving normally.

Emit an alarm (`STAT_SOFT_LIMIT_EXCEEDED`) only if the machine position itself (not just the planned endpoint) crosses the envelope — same convention as the JS driver's `_atBoundary` logic.

## Feedhold / E-stop / queue flush

- **Feedhold (`!`):** existing planner-side handling stops queued motion. Our cycle stays in `CYCLE_JGV` but pauses queueing new segments (planner is held). On resume, segment generation resumes from the still-current `v_target`. **No** automatic reset of velocities — the user's stick position drives whatever comes next.
- **Queue flush (`%`):** existing planner flush. Cycle exits via the standard `_jgv_finalize_exit` path. `v_target` is cleared.
- **Alarm / shutdown / panic:** cycle exits, all `v_target` cleared, no further segments queued.

## File layout

```
g2core/
  cycle_jgv.cpp                 (new — mirrors cycle_jogging.cpp pattern)
  canonical_machine.h            (add CYCLE_JGV enum, jogv state in cm struct)
  canonical_machine.cpp          (add cm_set_jogv setter, jgv_config_1 cfgSubtable)
  config_app.cpp                 (register jgv_config_1)
```

Minimal touch to `mp_exec_move()`, planner, or stepper code — all motion stays on the existing `cm_straight_feed_mm` → planner → exec path. The only new piece is the cycle that decides *what* to queue.

## State added to `cm` struct

```cpp
struct cm_jgv_state {
    bool       active;                 // CYCLE_JGV currently running
    float      v_target[AXES];         // commanded velocities (mm/min, signed)
    float      v_current[AXES];        // current jerk-limited velocities
    uint32_t   last_msg_time;          // for watchdog (SysTickHandler ms)
    // saved gcode model state (matches struct jmJoggingSingleton)
    uint8_t    saved_coord_system;
    uint8_t    saved_distance_mode;
    uint8_t    saved_feed_rate_mode;
    float      saved_feed_rate;
};
```

## Host-side changes (preview — for phase 5)

In `fabmo/g2.js`:
- New `G2.prototype.jogVelocity(vector, callback)` — sends `{"jgv":{...}}`, optionally awaits ack
- New `G2.prototype.jogStop(callback)` — sends `{"jgv":null}`, awaits exit notification

In `fabmo/runtime/manual/driver.js`:
- When `hasRatios` is true (analog motion), bypass the G1-segment streaming path entirely and call `g2.jogVelocity()` with per-axis velocities computed from ratios × max speed.
- Cardinal-axis button jogs (no ratios) continue to use the existing path — no behavior change for the keypad UI.

In `fabmo/pendant/devices/logitech-f310/index.js`:
- Drop the `startMotion`/`renew`/`pendingStart` state machine entirely; pendant just translates stick state to `{"jgv":...}` and ships it. Watchdog protects against pendant disconnect.

## Design decisions (locked 2026-05-23)

1. **JSON name:** `"jgv"` — terse, suited for high-frequency updates.
2. **Watchdog:** 500 ms default, configurable via `{"jgvto":ms}` ($-style persistent setting).
3. **Units mode:** honor `cm->gm.units_mode` (`gun`) — caller sends values in whatever units g2 is currently in (mm/min if G21, in/min if G20). Internally we convert to mm/min on entry.
4. **Spindle / coolant:** unchanged (left alone, like the existing position jog).
5. **Status report exposure:** none for v1 — `v_current[]` is internal. Can be added later if dashboards need it.
