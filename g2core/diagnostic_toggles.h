/*
 * diagnostic_toggles.h - compile-time switches for regression isolation
 *
 * These toggles are intended for temporary A/B testing while tracking
 * motion-quality regressions. Defaults preserve the v101.57.52+ behavior.
 *
 * Override at build time, e.g.:
 *   make CONFIG=sbv300 USER_DEFINES="-DG2_DIAG_ENABLE_RUNTIME_COMMAND_SR=0"
 *
 * NOTE: G2_DIAG_USE_DEFERRED_LOAD_MOVE has been resolved and removed.
 * st_request_load_move() is now always immediate; ESC spindle systick uses
 * st_request_load_move_deferred() which is drained by st_check_load_move().
 */

#ifndef DIAGNOSTIC_TOGGLES_H_ONCE
#define DIAGNOSTIC_TOGGLES_H_ONCE

// 1 = request an immediate full SR after runtime command callbacks
// 0 = disable this extra request path
#ifndef G2_DIAG_ENABLE_RUNTIME_COMMAND_SR
#define G2_DIAG_ENABLE_RUNTIME_COMMAND_SR 1
#endif

// 1 = run nested-hold synthetic two-report sequence
// 0 = disable sequence generation path
#ifndef G2_DIAG_ENABLE_NESTED_HOLD_SR_SEQUENCE
#define G2_DIAG_ENABLE_NESTED_HOLD_SR_SEQUENCE 1
#endif

// 1 = keep feedhold debug printf output enabled
// 0 = compile out feedhold debug printf calls
#ifndef G2_DIAG_ENABLE_FEEDHOLD_DEBUG_PRINTS
#define G2_DIAG_ENABLE_FEEDHOLD_DEBUG_PRINTS 1
#endif

#endif // DIAGNOSTIC_TOGGLES_H_ONCE
