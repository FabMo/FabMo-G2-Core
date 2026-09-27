# FabMo-G2-Core — notes for Claude

ShopBot/FabMo's fork of Synthetos **g2core** (C++ motion firmware, `main`
branch, firmware build 101.57.x). Split off 5–6 years ago; Synthetos is no
longer active on it. It runs on the ShopBot G2 controller board
(`g2core/board/…`, ShopBot settings in `g2core/settings/settings_shopbot_*.h`)
and is the other half of every FabMo motion behavior. See ~/.claude/CLAUDE.md
for the platform picture.

## Build and flash (current reality, Sept 2026)

- Source is edited on Ted's PC in VS Code but **compiled in Atmel Studio on a
  separate legacy PC** with JTAG debugging. There is a Makefile / `boards.mk`
  path (arm-none-eabi) that is the intended future route on the Pi; it is not
  yet the proven one. Do not assume a change can be built or tested from this
  Pi.
- Built binaries are copied into the engine repo `FabMo-Engine/firmware/`
  (`g2core_101.57.xx.bin`) and flashed from the engine via BOSSA.
- Physical test on a tool is mandatory for any firmware change. There is no
  simulator in use.

## Things the engine depends on

- **Status reports** (`report.cpp/.h`, `cfgArray` in `config_app.cpp`): the
  engine requests a fixed list of fields (`FabMo-Engine/config/g2_config.js`).
  `NV_STATUS_REPORT_LEN` (= `NV_MAX_OBJECTS`, `report.h`) caps the list; it
  was sized to exactly the FabMo list, so adding a field means raising it.
  Reports go out at `si` = 100 ms and only include changed fields (they
  "latch" on the engine side).
- `stat` values (0–9) and the quit/hold/flush sequence in the engine's `g2.js`
  are tuned to this firmware's actual behavior, including known quirks around
  job kill. Changing state transitions here can break the engine's workarounds.
- Spindle control (`spindle.cpp`, `spc`, `spph`/`spde` hold behavior),
  outputs/inputs (`gpio.*`, up to out12/in15 used), PWM (laser), feed override
  `fro`, motion mode `momo`.
- JSON command syntax and `{sr:…}`, `{jv:4}`, `{qv:0}` settings.

## Conventions

- Keep diffs minimal and local; this is a large upstream codebase we do not
  want to diverge from more than necessary.
- Note every engine-facing change (status fields, stat codes, JSON keys,
  defaults in settings_shopbot_*) in the PR so the engine can be updated in
  step.
- Open questions tracked as engine issues: input shaping feasibility, extra
  LEDs on the next board (PA21/PA30/PA18/PA19), `{spph:false}` not disabling
  retract in feedhold.

## Do not, without an explicit ask

- Change planner, jerk, or feedhold/cycle code (`plan_*.cpp`, `cycle_*.cpp`).
- Change status-report field semantics or `stat` meanings.
- Reformat upstream files.
