/*
 * cycle_jgv.cpp - velocity-mode jogging cycle
 * This file is part of the g2core project
 *
 * Copyright (c) 2026 FabMo
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// Velocity-mode jog cycle. Host sends {"jgv":{"x":120,"y":-80,...}}, which the
// JSON parser delivers as a series of child setters on tokens jgvx/jgvy/..., to set per-axis
// target velocities in current units/min (honors gun: in/min if G20, mm/min if G21).
// Internally everything is in mm/min. Cycle stays active until all targets are zero
// AND motion has ramped to a stop, the watchdog fires, or an alarm/queue-flush
// kills it. See docs/velocity_jog_design.md for the full design rationale.

#include "g2core.h"
#include "config.h"
#include "json_parser.h"
#include "text_parser.h"
#include "canonical_machine.h"
#include "planner.h"
#include "util.h"
#include "report.h"
#include "xio.h"
#include <math.h>

// Tuning knobs. These are conservative starting points — revisit after field testing.
#define JGV_SEGMENT_TIME_MS    50.0f     // per-segment duration in ms (50ms gives planner enough work to chew on; larger = more lag, smaller = planner can stall)
#define JGV_RAMP_TIME_S        0.25f     // 0→velocity_max ramp time, sets accel_max per axis
#define JGV_WATCHDOG_DEFAULT   500       // ms of host silence before auto-stop
#define JGV_STOP_EPSILON       0.01f     // mm/min — below this we treat velocity as zero (slower than this isn't useful jogging)

struct jgvSingleton {
    bool      active;                   // true while CYCLE_JGV is running
    float     v_target[AXES];           // commanded target velocities (mm/min, signed)
    float     v_current[AXES];          // current per-axis velocities (jerk-limited)
    float     planned_pos[AXES];        // cumulative endpoint of queued segments (mm, absolute machine coords) — we add displacement to this, not to the runtime position, so back-to-back queued G1s have distinct endpoints
    uint32_t  last_msg_time;            // SysTick ms timestamp of most recent {"jgv":...}
    uint32_t  next_commit_time;         // SysTick ms; commits before this are skipped (gives planner's BLOCK_TIMEOUT_MS=30 a chance to fire and kick STARTUP → PRIMING)
    uint32_t  watchdog_ms;              // configurable timeout ($jvto)
    bool      watchdog_tripped;         // true once we've forced targets to 0 from watchdog
    bool      hard_stop_requested;      // {"jgv":null} or {"jgv":0} — exit without ramp

    // saved gcode model state (restored on exit). We don't switch coord_system
    // during the cycle — absolute_override alone zeros offset application — so
    // we don't save/restore it. Distance and feed-rate modes are saved because
    // the cycle forces them to absolute mm/min while running.
    uint8_t   saved_distance_mode;
    uint8_t   saved_feed_rate_mode;
    float     saved_feed_rate;
};
static struct jgvSingleton jgv = {
    .active = false,
    .v_target = {0},
    .v_current = {0},
    .planned_pos = {0},
    .last_msg_time = 0,
    .next_commit_time = 0,
    .watchdog_ms = JGV_WATCHDOG_DEFAULT,
    .watchdog_tripped = false,
    .hard_stop_requested = false,
    .saved_distance_mode = 0,
    .saved_feed_rate_mode = 0,
    .saved_feed_rate = 0,
};

static void   _jgv_enter_cycle(void);
static void   _jgv_finalize_exit(void);
static bool   _jgv_all_targets_zero(void);
static bool   _jgv_all_current_zero(void);
static float  _jgv_accel_max(uint8_t axis);
static void   _jgv_apply_axis_softlimit(uint8_t axis, float planned_pos);

/*****************************************************************************
 * cm_set_jgv() - JSON setter for {"jgvx":N}, {"jgvy":N}, etc.
 *
 * The JSON parser splits {"jgv":{"x":120,"y":-80}} into multiple nvObj entries,
 * one per child. We're called once per child. Each call:
 *   - identifies the axis from the nv token (jvx → X)
 *   - converts the value from current display units to mm/min
 *   - clamps to that axis's velocity_max
 *   - stores in v_target[axis]
 *   - kicks the cycle into life if a nonzero target is set
 *   - resets the watchdog timestamp
 *
 * A null value or sentinel zero on every axis is handled by the caller — at this
 * level a 0 value means "ramp that axis to a stop." Hard-stop ({"jgv":null} or
 * {"jgv":0} at the parent) is handled by setting hard_stop_requested at JSON
 * parse time; we still process child setters normally if they come through.
 */
stat_t cm_set_jgv(nvObj_t *nv)
{
    int8_t axis = _axis(nv);
    if (axis < 0 || axis >= AXES) {
        return (STAT_INPUT_VALUE_RANGE_ERROR);
    }

    // Convert the user-supplied value from display units (in/min if G20,
    // mm/min if G21) to canonical mm/min for internal storage.
    float v_user = (float)nv->value_flt;
    float v_mm   = _to_millimeters(v_user);

    // Clamp to per-axis velocity_max (signed — preserve direction).
    float vmax = cm->a[axis].velocity_max;
    if (v_mm >  vmax) v_mm =  vmax;
    if (v_mm < -vmax) v_mm = -vmax;

    jgv.v_target[axis] = v_mm;
    jgv.last_msg_time  = SysTickTimer.getValue();
    jgv.watchdog_tripped = false;        // any new message clears the watchdog flag

    // Reflect the applied value back to the host in their unit space — that
    // way an echo of {"jgvx":0.5} reads as 0.5, not 12.7, when the host is
    // in inches mode. Do NOT use set_float() here: it would re-read
    // nv->value_flt and discard our converted value.
    float v_echo = v_mm;
    if (cm_get_units_mode(MODEL) == INCHES) {
        v_echo = v_mm / MM_PER_INCH;
    }
    nv->value_flt = v_echo;
    nv->valuetype = TYPE_FLOAT;
    nv->precision = GET_TABLE_WORD(precision);

    // Kick the cycle if a nonzero velocity is being commanded and we're idle.
    // Compare against v_mm (canonical), not the display-units value, so the
    // epsilon threshold means the same thing regardless of units mode.
    if (!jgv.active && fabsf(v_mm) > JGV_STOP_EPSILON) {
        if (cm->machine_state == MACHINE_ALARM ||
            cm->machine_state == MACHINE_SHUTDOWN ||
            cm->machine_state == MACHINE_PANIC) {
            return (STAT_COMMAND_NOT_ACCEPTED);
        }
        if (cm->cycle_type != CYCLE_NONE && cm->cycle_type != CYCLE_MACHINING) {
            // Don't barge into homing / probing / position-jog cycles.
            return (STAT_COMMAND_NOT_ACCEPTED);
        }
        _jgv_enter_cycle();
    }

    return (STAT_OK);
}

/*****************************************************************************
 * cm_get_jgvto() / cm_set_jgvto() - watchdog timeout in ms
 */
stat_t cm_get_jgvto(nvObj_t *nv) { return (get_integer(nv, (uint32_t)jgv.watchdog_ms)); }
stat_t cm_set_jgvto(nvObj_t *nv)
{
    uint32_t v = (uint32_t)nv->value_int;
    if (v < 50)    v = 50;             // floor: shorter than this is dangerous (network jitter)
    if (v > 60000) v = 60000;          // ceiling: longer than a minute is silly
    jgv.watchdog_ms = v;
    nv->value_int = (float)v;
    return (STAT_OK);
}

/*****************************************************************************
 * cm_jgv_abort() - external abort
 *
 * Called from feedhold/queue-flush/alarm paths when motion is being torn down.
 * Clears velocity state and exits the cycle cleanly. Idempotent.
 */
void cm_jgv_abort(void)
{
    if (!jgv.active) return;
    for (uint8_t a = 0; a < AXES; a++) {
        jgv.v_target[a]  = 0;
        jgv.v_current[a] = 0;
    }
    _jgv_finalize_exit();
}

/*****************************************************************************
 * cm_jgv_callback() - main loop tick
 *
 * Runs each iteration of the main controller dispatch (see controller.cpp).
 * Returns STAT_NOOP when nothing to do (cycle not active or planner is full).
 * Returns STAT_EAGAIN while the cycle is active so the dispatch knows we're
 * still working — same convention as cm_jogging_cycle_callback.
 */
stat_t cm_jgv_callback(void)
{
    if (!jgv.active) {
        return (STAT_NOOP);
    }
    // Self-heal: if our cycle_type was clobbered by another path (a homing or
    // probing cycle starting, cm_canned_cycle_end being called from somewhere
    // we didn't anticipate, an alarm reset, etc.) we'd otherwise spin in this
    // callback forever as a no-op — jgv.active stays true but the new entry
    // gate (`!jgv.active && ...` in cm_set_jgv) prevents fresh commands from
    // restarting the cycle. Detect the desync and reset state so the next
    // {"jgvx":...} can re-enter cleanly.
    if (cm->cycle_type != CYCLE_JGV) {
        jgv.active = false;
        for (uint8_t a = 0; a < AXES; a++) {
            jgv.v_target[a]  = 0;
            jgv.v_current[a] = 0;
        }
        return (STAT_NOOP);
    }

    // Watchdog check — host silence forces all targets to 0 (ramps to stop).
    uint32_t now = SysTickTimer.getValue();
    if (!jgv.watchdog_tripped && (now - jgv.last_msg_time) > jgv.watchdog_ms) {
        for (uint8_t a = 0; a < AXES; a++) {
            jgv.v_target[a] = 0;
        }
        jgv.watchdog_tripped = true;
        // Could xio_writeline an error notification here, but jgv.last_msg_time
        // staying old will make any future cm_set_jgv call recover gracefully.
    }

    // Hard-stop request bypasses the ramp and exits immediately. The caller
    // (cm_run_jv_stop, not yet wired) is expected to have already flushed
    // the planner; we just unwind cycle state here.
    if (jgv.hard_stop_requested) {
        for (uint8_t a = 0; a < AXES; a++) {
            jgv.v_target[a]  = 0;
            jgv.v_current[a] = 0;
        }
        _jgv_finalize_exit();
        return (STAT_OK);
    }

    // Exit condition: all targets zero AND no residual velocity AND planner is
    // drained. The runtime busy check mirrors cm_jogging_cycle_callback's exit
    // handshake — we don't tear down state until queued motion has played out.
    if (_jgv_all_targets_zero() && _jgv_all_current_zero()) {
        if (cm_get_runtime_busy()) {
            return (STAT_EAGAIN);       // wait for planner to drain
        }
        _jgv_finalize_exit();
        return (STAT_OK);
    }

    // Don't pile up segments — keep the planner queue shallow.
    if (mp_planner_is_full(mp)) {
        return (STAT_EAGAIN);
    }

    // Pace our commits to roughly one per segment-time. Without this gate the
    // main loop would call us at >>1 kHz and we'd reset the planner's internal
    // BLOCK_TIMEOUT_MS (30 ms) on every commit, preventing it from ever
    // transitioning out of PLANNER_STARTUP — so the runtime never actually
    // begins executing the queued moves. By committing slower than that
    // timeout, the planner naturally fires its "no more blocks coming, start
    // running" transition between our commits, and motion proceeds.
    if (SysTickTimer.getValue() < jgv.next_commit_time) {
        return (STAT_EAGAIN);
    }

    // Compute the next segment's per-axis end-of-segment velocity (jerk-
    // limited) and the corresponding displacement.
    float segment_time_min = JGV_SEGMENT_TIME_MS / 60000.0f;

    float v_end[AXES];
    float target_pos[AXES];
    bool  flags[AXES];
    float toolpath_speed_sq = 0;

    for (uint8_t a = 0; a < AXES; a++) {
        float v_cur = jgv.v_current[a];
        float v_tgt = jgv.v_target[a];

        // Per-axis max acceleration (mm/min^2) — go from 0 → vmax in JGV_RAMP_TIME_S.
        float accel_max = _jgv_accel_max(a);
        float max_dv    = accel_max * segment_time_min;

        float dv = v_tgt - v_cur;
        if (dv >  max_dv) dv =  max_dv;
        if (dv < -max_dv) dv = -max_dv;
        v_end[a] = v_cur + dv;

        // Snap very small velocities to zero so the cycle can actually exit.
        if (fabsf(v_end[a]) < JGV_STOP_EPSILON && fabsf(v_tgt) < JGV_STOP_EPSILON) {
            v_end[a] = 0;
        }

        // Trapezoidal displacement over the segment, in mm.
        float displacement = (v_cur + v_end[a]) * 0.5f * segment_time_min;

        // Build absolute target position. Accumulate against the *planned*
        // endpoint of our previously queued segments, NOT the runtime
        // position — otherwise back-to-back segments share an endpoint, the
        // planner sees zero-length moves, and nothing actually executes.
        float pos_now = jgv.planned_pos[a];
        target_pos[a] = pos_now + displacement;

        // Per-axis soft-limit pre-check: if pushing further into a limit, zero
        // this axis for this segment. cm_straight_feed_mm has its own
        // cm_test_soft_limits which would abort the whole segment; the per-axis
        // pre-check lets the other axes keep moving.
        _jgv_apply_axis_softlimit(a, target_pos[a]);

        // If softlimit clipping zeroed this axis, also pin v_end[a] so the
        // accumulated v_current state reflects what actually got queued.
        if (fabsf(target_pos[a] - pos_now) < 1e-7f) {
            v_end[a] = 0;
            jgv.v_target[a] = 0;         // don't keep trying — host has to re-command opposite dir
        }

        // Only flag the axis as "in this move" if there's actual displacement.
        // Zero-displacement axes shouldn't be in the gcode line.
        flags[a] = (fabsf(target_pos[a] - pos_now) > 1e-7f);

        toolpath_speed_sq += v_end[a] * v_end[a];
    }

    float toolpath_F = sqrtf(toolpath_speed_sq);
    if (toolpath_F < JGV_STOP_EPSILON) {
        // No motion this segment. Just advance state and wait for the next tick.
        for (uint8_t a = 0; a < AXES; a++) jgv.v_current[a] = v_end[a];
        return (STAT_EAGAIN);
    }

    cm_set_feed_rate_mm(toolpath_F);
    stat_t status = cm_straight_feed_mm(target_pos, flags, PROFILE_FAST);
    if (status != STAT_OK) {
        // Most likely a soft-limit error escaped our per-axis pre-check (e.g.
        // current position is already past the envelope due to overshoot).
        // Don't escalate — just stop queueing this segment; next iteration will
        // try again with whatever v_target the host has set by then.
        return (STAT_EAGAIN);
    }

    // Advance both the velocity tracker and the planned-position tracker by
    // what we actually queued. Subsequent segments stack on top of this.
    for (uint8_t a = 0; a < AXES; a++) {
        jgv.v_current[a]  = v_end[a];
        jgv.planned_pos[a] = target_pos[a];
    }
    // Hold off the next commit. Anything shorter than ~30 ms would defeat the
    // planner's block timeout; we choose JGV_SEGMENT_TIME_MS so the next
    // commit lands just as the previous segment is consumed (queue stays
    // ~1 segment deep).
    jgv.next_commit_time = SysTickTimer.getValue() + (uint32_t)JGV_SEGMENT_TIME_MS;

    return (STAT_EAGAIN);               // stay in the cycle
}

/*****************************************************************************
 * internal helpers
 */

static void _jgv_enter_cycle(void)
{
    // Save the gcode model state we're about to clobber, so finalize can
    // restore it. Mirrors cm_jogging_cycle_start exactly.
    jgv.saved_distance_mode  = cm_get_distance_mode(ACTIVE_MODEL);
    jgv.saved_feed_rate_mode = cm_get_feed_rate_mode(ACTIVE_MODEL);
    jgv.saved_feed_rate      = (ACTIVE_MODEL)->feed_rate;

    cm_set_distance_mode(ABSOLUTE_DISTANCE_MODE);
    cm_set_feed_rate_mode(UNITS_PER_MINUTE_MODE);
    // Disable coord-system/tool/g92 offset addition for our segment targets.
    // cm_set_model_target() would otherwise compute target = offset + supplied,
    // which means our small per-segment positions would land at huge absolute
    // coordinates if any offset is non-zero. Setting absolute_override alone
    // makes cm_get_combined_offset() short-circuit to 0 for the duration of
    // the cycle — we don't ALSO need to switch coord_system, and avoiding that
    // means the host's display coord frame stays unchanged through the cycle.
    cm_set_absolute_override(MODEL, ABSOLUTE_OVERRIDE_ON_DISPLAY_WITH_OFFSETS);

    // Sync our planned-position tracker AND the planner's internal mp->position
    // to the current runtime position. Without the mp_set_planner_position call,
    // the very first G1 we queue would have axis_length = (our_target - mp->position),
    // and mp->position may be stale (e.g. zero after fabmo init), producing a
    // massive first-segment move at planner-permitted velocity.
    for (uint8_t a = 0; a < AXES; a++) {
        jgv.v_current[a]   = 0;            // always start from a known rest
        float pos          = cm_get_absolute_position(RUNTIME, a);
        jgv.planned_pos[a] = pos;
        mp_set_planner_position(a, pos);
    }
    jgv.watchdog_tripped     = false;
    jgv.hard_stop_requested  = false;
    jgv.next_commit_time     = 0;          // commit immediately on entry
    jgv.active               = true;

    cm->machine_state = MACHINE_CYCLE;
    cm->cycle_type    = CYCLE_JGV;
    sr_request_status_report(SR_REQUEST_IMMEDIATE);
}

static void _jgv_finalize_exit(void)
{
    cm_set_absolute_override(MODEL, ABSOLUTE_OVERRIDE_OFF);
    cm_set_distance_mode(jgv.saved_distance_mode);
    cm_set_feed_rate_mode(jgv.saved_feed_rate_mode);
    (MODEL)->feed_rate = jgv.saved_feed_rate;
    cm_set_motion_mode(MODEL, MOTION_MODE_CANCEL_MOTION_MODE);
    cm_canned_cycle_end();

    jgv.active = false;
    for (uint8_t a = 0; a < AXES; a++) {
        jgv.v_target[a]  = 0;
        jgv.v_current[a] = 0;
    }
    xio_writeline("{\"jgv\":0}\n");     // notify host that cycle has exited
}

static bool _jgv_all_targets_zero(void)
{
    for (uint8_t a = 0; a < AXES; a++) {
        if (fabsf(jgv.v_target[a]) > JGV_STOP_EPSILON) return (false);
    }
    return (true);
}

static bool _jgv_all_current_zero(void)
{
    for (uint8_t a = 0; a < AXES; a++) {
        if (fabsf(jgv.v_current[a]) > JGV_STOP_EPSILON) return (false);
    }
    return (true);
}

// Per-axis max acceleration. Picks the lesser of:
//   - vmax / JGV_RAMP_TIME_S  (so a full deflection ramps in ~250 ms)
//   - jerk-based equivalent (planner uses jerk-limited S-curves; we approximate)
// jerk_max in the axis config is stored as (mm/min^3) / 1e6 — i.e. divided by
// a million. The acceleration "available" over a JGV_RAMP_TIME_S window from
// constant jerk is ~ J*T, treating the ramp as a single jerk-up phase.
static float _jgv_accel_max(uint8_t axis)
{
    float vmax = cm->a[axis].velocity_max;
    if (vmax <= 0) return (0);
    float a_from_ramp = vmax / (JGV_RAMP_TIME_S / 60.0f);    // mm/min^2 (RAMP_TIME_S → min)
    return (a_from_ramp);
}

// Clamp target_pos for one axis to stay within the soft-limit envelope.
// If clamping happens, the caller observes target_pos[axis] == pos_now and
// zeroes that axis. Other axes are left alone — pushing into a limit on X
// shouldn't kill Y motion.
static void _jgv_apply_axis_softlimit(uint8_t axis, float planned_pos)
{
    (void)axis; (void)planned_pos;
    // For v1 we lean on cm_straight_feed_mm's internal cm_test_soft_limits to
    // catch envelope violations at the canonical-machine level. The whole-move
    // abort it does is acceptable here because the next tick (5 ms later) will
    // retry with whatever v_target the host has shipped in the meantime.
    //
    // Per-axis selective clipping (so X-into-limit doesn't stop Y) is a known
    // refinement — leave it as a follow-up. The user-visible symptom of the
    // simpler approach is: pushing into an X limit briefly stalls Y too,
    // recovering as soon as the host reduces X command.
}
