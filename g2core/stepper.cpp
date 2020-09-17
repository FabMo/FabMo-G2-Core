/*
 * stepper.cpp - stepper motor controls
 * This file is part of the g2core project
 *
 * Copyright (c) 2010 - 2019 Alden S. Hart, Jr.
 * Copyright (c) 2013 - 2019 Robert Giseburt
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
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
/*  This module provides the low-level stepper drivers and some related functions.
 *  See stepper.h for a detailed explanation of this module.
 */

#include "g2core.h"
#include "config.h"
#include "stepper.h"
#include "encoder.h"
#include "planner.h"
#include "hardware.h"
#include "text_parser.h"
#include "util.h"
#include "controller.h"
#include "xio.h"
#include "kinematics.h"
#include "gpio.h"

#include "spindle.h"

/**** Debugging output with semihosting ****/

#include "MotateDebug.h"

/* Note: stepper_debug statements removed 1/16/17 in SHA eb0905ccae03c04f99e6f471cbe029002f0324c6.
 * See earlier commits to recover
 */

/**** Allocate structures ****/

stConfig_t st_cfg HOT_DATA;
stPrepSingleton_t st_pre HOT_DATA;
static stRunSingleton_t st_run HOT_DATA;

/**** Static functions ****/

static void _load_move(void) HOT_FUNC;

/**** Setup motate ****/

extern OutputPin<Motate::kDebug1_PinNumber> debug_pin1;
extern OutputPin<Motate::kDebug2_PinNumber> debug_pin2;
extern OutputPin<Motate::kDebug3_PinNumber> debug_pin3;
//extern OutputPin<Motate::kDebug4_PinNumber> debug_pin4;

dda_timer_type dda_timer     {Motate::kTimerUpToMatch, FREQUENCY_DDA};      // stepper pulse generation
exec_timer_type exec_timer;         // triggers calculation of next+1 stepper segment
fwd_plan_timer_type fwd_plan_timer; // triggers planning of next block

// SystickEvent for handling dwells (must be registered before it is active)
Motate::SysTickEvent dwell_systick_event{
    [] {
        // we're either in a dewll or a spindle speed ramp "dwell"
        // in either case, if a feedhold comes in, we need to bail, and since the dwell *is* the motion
        // move the state machine along from here
        if (cm->hold_state == FEEDHOLD_SYNC) {
            st_run.dwell_ticks_downcount = 1; // this'll decerement to zero shortly
            cm->hold_state = FEEDHOLD_MOTION_STOPPED;
        }
        if ((--st_run.dwell_ticks_downcount == 0)) {
            st_run.dwell_ticks_downcount = 0;  // in the case of stop==true, this is needed
            SysTickTimer.unregisterEvent(&dwell_systick_event);
            _load_move();  // load the next move at the current interrupt level
        }
    },
    nullptr};

/* Note on the above:
It's a lambda function creating a closure function.
The full implementation that uses it is small and may help:
https://github.com/synthetos/Motate/blob/41e5b92a98de4b268d1804bf6eadf3333298fc75/MotateProject/motate/Atmel_sam_common/SamTimers.h#L1147-L1218
It's just like a function, and is used as a function pointer.

But the closure part means that whatever variables that were in scope where the
[&](parameters){code} is will be captured by the compiler as references in the generated
function and used wherever the function gets called. In this particular use, there isn't
anything that wouldn't be available anywhere in that file, but they're not being called
from that file. They're being called by the systick interrupt which is over in SamTmers.cpp
So this saves a bunch of work exposing bits that the systick would need to call and encapsulates it.
And there's almost no runtime overhead. Just a check for a valid function pointer and then a call of it.
I'd like to get rid of that check but it's more work than its worth.

See here for some good info on lambda functions in C++
http://www.cprogramming.com/c++11/c++11-lambda-closures.html
http://en.cppreference.com/w/cpp/language/lambda
*/

/************************************************************************************
 **** CODE **************************************************************************
 ************************************************************************************/
/*
 * stepper_init() - initialize stepper motor subsystem
 * stepper_reset() - reset stepper motor subsystem
 *
 *  Notes:
 *    - This init requires sys_init() to be run beforehand
 *    - microsteps are setup during config_init()
 *    - motor polarity is setup during config_init()
 *    - high level interrupts must be enabled in main() once all inits are complete
 */
/*  NOTE: This is the bare code that the Motate timer calls replace.
 *  NB: requires: #include <component_tc.h>
 *
 *  REG_TC1_WPMR = 0x54494D00;              // enable write to registers
 *  TC_Configure(TC_BLOCK_DDA, TC_CHANNEL_DDA, TC_CMR_DDA);
 *  REG_RC_DDA = TC_RC_DDA;                 // set frequency
 *  REG_IER_DDA = TC_IER_DDA;               // enable interrupts
 *  NVIC_EnableIRQ(TC_IRQn_DDA);
 *  pmc_enable_periph_clk(TC_ID_DDA);
 *  TC_Start(TC_BLOCK_DDA, TC_CHANNEL_DDA);
 */
void stepper_init()
{
    using namespace Motate;

    memset(&st_run, 0, sizeof(st_run));            // clear all values, pointers and status
    memset(&st_pre, 0, sizeof(st_pre));            // clear all values, pointers and status
    stepper_init_assertions();

    // setup DDA timer
    // Longer duty cycles stretch ON pulses but 75% is about the upper limit and about
    // optimal for 200 KHz DDA clock before the time in the OFF cycle is too short.
    // If you need more pulse width you need to drop the DDA clock rate
    dda_timer.setInterrupts(kInterruptOnOverflow | kInterruptPriorityHighest);

    // setup software interrupt exec timer & initial condition
    exec_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityHigh);
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_EXEC;

    // setup software interrupt forward plan timer & initial condition
    fwd_plan_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityMedium);

    board_stepper_init();
    stepper_reset();                            // reset steppers to known state

    // setup motor power levels and apply power level to stepper drivers
    for (uint8_t motor=0; motor<MOTORS; motor++) {
        Motors[motor]->setPowerLevels(st_cfg.mot[motor].power_level, st_cfg.mot[motor].power_level_idle);
    }

    dda_timer.start();                          // start the DDA timer if not already running
}

/*
 * stepper_reset() - reset stepper internals
 *
 * Used to initialize stepper and also to halt movement
 */

void stepper_reset()
{
    dda_timer.stop();                                   // stop all movement
    st_run.dda_ticks_downcount = 0;                     // signal the runtime is not busy
    st_run.dwell_ticks_downcount = 0;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_EXEC;    // set to EXEC or it won't restart

    for (uint8_t motor=0; motor<MOTORS; motor++) {
        st_pre.mot[motor].prev_direction = STEP_INITIAL_DIRECTION;
        st_pre.mot[motor].direction = STEP_INITIAL_DIRECTION;
        st_run.mot[motor].substep_accumulator = 0;      // will become max negative during per-motor setup;
        st_pre.mot[motor].corrected_steps = 0;          // diagnostic only - no action effect
    }
    mp_set_steps_to_runtime_position();                 // reset encoder to agree with the above
}

/*
 * stepper_init_assertions() - test assertions, return error code if violation exists
 * stepper_test_assertions() - test assertions, return error code if violation exists
 */

void stepper_init_assertions()
{
    st_run.magic_end = MAGICNUM;
    st_run.magic_start = MAGICNUM;
    st_pre.magic_end = MAGICNUM;
    st_pre.magic_start = MAGICNUM;
}

stat_t stepper_test_assertions()
{
    if ((BAD_MAGIC(st_run.magic_start)) || (BAD_MAGIC(st_run.magic_end)) ||
        (BAD_MAGIC(st_pre.magic_start)) || (BAD_MAGIC(st_pre.magic_end))) {
        return(cm_panic(STAT_STEPPER_ASSERTION_FAILURE, "stepper_test_assertions()"));
    }
    return (STAT_OK);
}

/*
 * st_runtime_isbusy() - return TRUE if runtime is busy:
 *
 *  Busy conditions:
 *  - motors are running
 *  - dwell is running
 *  - a toolhead is busy in a way that should prevent motion (spinup, etc.)
 */

bool st_runtime_isbusy()
{
    // note: zero is false, anything else is true
    return (st_run.dda_ticks_downcount || st_run.dwell_ticks_downcount || is_a_toolhead_busy());
}

/*
 * st_clc() - clear counters
 */

stat_t st_clc(nvObj_t *nv)    // clear diagnostic counters, reset stepper prep
{
    stepper_reset();
    return(STAT_OK);
}

/*
 * st_motor_power_callback() - callback to manage motor power sequencing
 *
 *  Handles motor power-down timing, low-power idle, and adaptive motor power
 */

stat_t st_motor_power_callback()     // called by controller
{
    if (!mp_is_phat_city_time()) {   // don't process this if you are time constrained in the planner
        return (STAT_NOOP);
    }

    bool have_actually_stopped = false;
    if ((!st_runtime_isbusy()) &&
        (st_pre.buffer_state != PREP_BUFFER_OWNED_BY_LOADER) &&
        (cm_get_machine_state() != MACHINE_CYCLE)) {    // if there are no moves to load...
        have_actually_stopped = true;
    }

    // manage power for each motor individually
    for (uint8_t motor = MOTOR_1; motor < MOTORS; motor++) {
        Motors[motor]->periodicCheck(have_actually_stopped);
    }
    return (STAT_OK);
}

/******************************
 * Interrupt Service Routines *
 ******************************/

/***** Stepper Interrupt Service Routine ************************************************
 * ISR - DDA timer interrupt routine - service ticks from DDA timer
 */

/*
 *  The DDA timer interrupt does this:
 *    - fire on overflow
 *    - clear interrupt condition
 *    - clear all step pins - this clears those the were set during the previous interrupt
 *    - if downcount == 0 and stop the timer and exit
 *    - run the DDA for each channel
 *    - decrement the downcount - if it reaches zero load the next segment
 *
 *  Note that the motor_N.step.isNull() tests are compile-time tests, not run-time tests.
 *  If motor_N is not defined that if{} clause (i.e. that motor) drops out of the complied code.
 */

namespace Motate {            // Must define timer interrupts inside the Motate namespace
    template<>
    void dda_timer_type::interrupt() HOT_FUNC;

template<>
void dda_timer_type::interrupt()
{
    dda_timer.getInterruptCause();  // clear interrupt condition

    // clear all steps from the previous interrupt
    motor_1.stepEnd();
    motor_2.stepEnd();
#if MOTORS > 2
    motor_3.stepEnd();
#endif
#if MOTORS > 3
    motor_4.stepEnd();
#endif
#if MOTORS > 4
    motor_5.stepEnd();
#endif
#if MOTORS > 5
    motor_6.stepEnd();
#endif

    // process last DDA tick after end of segment
    if (st_run.dda_ticks_downcount == 0) {
        // we used to turn off the stepper timer here, but we don't anymore
        return;
    }

//  The following code would work, but it's faster on the M3 to loop unroll it. Perhaps not on the M7
//    for (uint8_t motor=0; motor<MOTORS; motor++) {
//        if  ((st_run.mot[motor].substep_accumulator += st_run.mot[motor].substep_increment) > 0) {
//            Motors[motor]->stepStart();        // turn step bit on
//            st_run.mot[motor].substep_accumulator -= DDA_SUBSTEPS;
//            INCREMENT_ENCODER(motor);
//        }
//        st_run.mot[motor].substep_increment += st_run.mot[motor].substep_increment_increment;
//    }

    // process DDAs for each motor
    if  ((st_run.mot[MOTOR_1].substep_accumulator += st_run.mot[MOTOR_1].substep_increment) > 0) {
        motor_1.stepStart();        // turn step bit on
        st_run.mot[MOTOR_1].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_1);
    }
    st_run.mot[MOTOR_1].substep_increment += st_run.mot[MOTOR_1].substep_increment_increment;
    if ((st_run.mot[MOTOR_2].substep_accumulator += st_run.mot[MOTOR_2].substep_increment) > 0) {
        motor_2.stepStart();        // turn step bit on
        st_run.mot[MOTOR_2].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_2);
    }
    st_run.mot[MOTOR_2].substep_increment += st_run.mot[MOTOR_2].substep_increment_increment;
#if MOTORS > 2
    if ((st_run.mot[MOTOR_3].substep_accumulator += st_run.mot[MOTOR_3].substep_increment) > 0) {
        motor_3.stepStart();        // turn step bit on
        st_run.mot[MOTOR_3].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_3);
    }
    st_run.mot[MOTOR_3].substep_increment += st_run.mot[MOTOR_3].substep_increment_increment;
#endif
#if MOTORS > 3
    if ((st_run.mot[MOTOR_4].substep_accumulator += st_run.mot[MOTOR_4].substep_increment) > 0) {
        motor_4.stepStart();        // turn step bit on
        st_run.mot[MOTOR_4].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_4);
    }
    st_run.mot[MOTOR_4].substep_increment += st_run.mot[MOTOR_4].substep_increment_increment;
#endif
#if MOTORS > 4
    if ((st_run.mot[MOTOR_5].substep_accumulator += st_run.mot[MOTOR_5].substep_increment) > 0) {
        motor_5.stepStart();        // turn step bit on
        st_run.mot[MOTOR_5].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_5);
    }
    st_run.mot[MOTOR_5].substep_increment += st_run.mot[MOTOR_5].substep_increment_increment;
#endif
#if MOTORS > 5
    if ((st_run.mot[MOTOR_6].substep_accumulator += st_run.mot[MOTOR_6].substep_increment) > 0) {
        motor_6.stepStart();        // turn step bit on
        st_run.mot[MOTOR_6].substep_accumulator -= DDA_SUBSTEPS;
        INCREMENT_ENCODER(MOTOR_6);
    }
    st_run.mot[MOTOR_6].substep_increment += st_run.mot[MOTOR_6].substep_increment_increment;
#endif

    // Process end of segment.
    // One more interrupt will occur to turn of any pulses set in this pass.
    if (--st_run.dda_ticks_downcount == 0) {
        _load_move();       // load the next move at the current interrupt level
    }
} // MOTATE_TIMER_INTERRUPT
} // namespace Motate

/****************************************************************************************
 * Exec sequencing code   - computes and prepares next load segment
 * st_request_exec_move() - SW interrupt to request to execute a move
 * exec_timer interrupt   - interrupt handler for calling exec function
 */

void st_request_exec_move()
{
    exec_timer.setInterruptPending();
}

namespace Motate {    // Define timer inside Motate namespace
    template<>
    void exec_timer_type::interrupt() HOT_FUNC;

    template<>
    void exec_timer_type::interrupt()
    {
        exec_timer.getInterruptCause();                    // clears the interrupt condition
        if (st_pre.buffer_state == PREP_BUFFER_OWNED_BY_EXEC) {
            if (mp_exec_move() != STAT_NOOP) {
                st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER; // flip it back
                st_request_load_move();
                return;
            }
        }
    }
} // namespace Motate

/****************************************************************************************
 * st_request_forward_plan  - performs forward planning on penultimate block
 * fwd_plan interrupt       - interrupt handler for calling forward planning function
 */

void st_request_forward_plan()
{
    fwd_plan_timer.setInterruptPending();
}

namespace Motate {    // Define timer inside Motate namespace
    template<>
    void fwd_plan_timer_type::interrupt() HOT_FUNC;

    template<>
    void fwd_plan_timer_type::interrupt()
    {
        fwd_plan_timer.getInterruptCause();     // clears the interrupt condition
        if (mp_forward_plan() != STAT_NOOP) {   // We now have a move to exec.
            st_request_exec_move();
            return;
        }
    }
} // namespace Motate

/****************************************************************************************
 * Loader sequencing code
 * st_request_load_move() - fires a software interrupt (timer) to request to load a move
 * load_move interrupt    - interrupt handler for running the loader
 *
 *  _load_move() can only be called be called from an ISR at the same or higher level as
 *  the DDA or dwell ISR. A software interrupt has been provided to allow a non-ISR to
 *  request a load (see st_request_load_move())
 */

void st_request_load_move()
{
    if (st_runtime_isbusy()) {                                      // don't request a load if the runtime is busy
        return;
    }
    if (st_pre.buffer_state == PREP_BUFFER_OWNED_BY_LOADER) {       // bother interrupting
       _load_move();
    }
}

/****************************************************************************************
 * _load_move() - Dequeue move and load into stepper runtime structure
 *
 *  This routine can only be called be called from an ISR at the same or
 *  higher level as the DDA or dwell ISR. A software interrupt has been
 *  provided to allow a non-ISR to request a load (st_request_load_move())
 *
 *  In aline() code:
 *   - All axes must set steps and compensate for out-of-range pulse phasing.
 *   - If axis has 0 steps the direction setting can be omitted
 *   - If axis has 0 steps the motor power must be set accord to the power mode
 */

static void _load_move()
{
    // Be aware that dda_ticks_downcount must equal zero for the loader to run.
    // So the initial load must also have this set to zero as part of initialization
    if (st_runtime_isbusy()) {
        return;                     // exit if the runtime is busy
    }

    // If there are no moves to load start motor power timeouts
    if (st_pre.buffer_state != PREP_BUFFER_OWNED_BY_LOADER) {
        motor_1.motionStopped();    // ...start motor power timeouts
        motor_2.motionStopped();
#if (MOTORS > 2)
        motor_3.motionStopped();
#endif
#if (MOTORS > 3)
        motor_4.motionStopped();
#endif
#if (MOTORS > 4)
        motor_5.motionStopped();
#endif
#if (MOTORS > 5)
        motor_6.motionStopped();
#endif
        return;
    } // if (st_pre.buffer_state != PREP_BUFFER_OWNED_BY_LOADER)

    // give the toolhead a chance to react to the upcoming move
    if (st_pre.bf) {
        spindle_engage(st_pre.bf->gm);
    }

    // handle aline loads first (most common case)
    if (st_pre.block_type == BLOCK_TYPE_ALINE) {

        //**** setup the new segment ****

        // st_run.dda_ticks_downcount is setup right before turning on the interrupt, since we don't turn it off
        // INLINED VERSION: 4.3us
        //**** MOTOR_1 LOAD ****

        // These sections are somewhat optimized for execution speed. The whole load operation
        // is supposed to take < 5 uSec (Arm M3 core). Be careful if you mess with this.

        // the following if() statement sets the runtime substep increment value or zeroes it
        if ((st_run.mot[MOTOR_1].substep_increment = st_pre.mot[MOTOR_1].substep_increment) != 0) {
            // NB: If motor has 0 steps the following is all skipped. This ensures that state comparisons
            //     always operate on the last segment actually run by this motor, regardless of how many
            //     segments it may have been inactive in between.

            // Prepare the substep increment increment for linear velocity ramping
            st_run.mot[MOTOR_1].substep_increment_increment = st_pre.mot[MOTOR_1].substep_increment_increment;

            // Detect direction change and if so:
            //    Set the direction bit in hardware.
            //    Compensate for direction change by flipping substep accumulator value about its midpoint.

            if (st_pre.mot[MOTOR_1].direction != st_pre.mot[MOTOR_1].prev_direction) {
                st_pre.mot[MOTOR_1].prev_direction = st_pre.mot[MOTOR_1].direction;
                st_run.mot[MOTOR_1].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_1].substep_accumulator); // invert the accumulator for the direction change
                motor_1.setDirection(st_pre.mot[MOTOR_1].direction);
            }

            // Enable the stepper and start/update motor power management
            motor_1.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_1, st_pre.mot[MOTOR_1].step_sign);

        } else {  // Motor has 0 steps; might need to energize motor for power mode processing
            st_run.mot[MOTOR_1].substep_increment_increment = 0;
            motor_1.motionStopped();
        }
        // accumulate counted steps to the step position and zero out counted steps for the segment currently being loaded
        ACCUMULATE_ENCODER(MOTOR_1);

#if (MOTORS >= 2)
        if ((st_run.mot[MOTOR_2].substep_increment = st_pre.mot[MOTOR_2].substep_increment) != 0) {
            st_run.mot[MOTOR_2].substep_increment_increment = st_pre.mot[MOTOR_2].substep_increment_increment;
            if (st_pre.mot[MOTOR_2].direction != st_pre.mot[MOTOR_2].prev_direction) {
                st_pre.mot[MOTOR_2].prev_direction = st_pre.mot[MOTOR_2].direction;
                st_run.mot[MOTOR_2].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_2].substep_accumulator); // invert the accumulator for the direction change
                motor_2.setDirection(st_pre.mot[MOTOR_2].direction);
            }
            motor_2.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_2, st_pre.mot[MOTOR_2].step_sign);
        } else {
            st_run.mot[MOTOR_2].substep_increment_increment = 0;
            motor_2.motionStopped();
        }
        ACCUMULATE_ENCODER(MOTOR_2);
#endif
#if (MOTORS >= 3)
        if ((st_run.mot[MOTOR_3].substep_increment = st_pre.mot[MOTOR_3].substep_increment) != 0) {
            st_run.mot[MOTOR_3].substep_increment_increment = st_pre.mot[MOTOR_3].substep_increment_increment;
            if (st_pre.mot[MOTOR_3].direction != st_pre.mot[MOTOR_3].prev_direction) {
                st_pre.mot[MOTOR_3].prev_direction = st_pre.mot[MOTOR_3].direction;
                st_run.mot[MOTOR_3].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_3].substep_accumulator); // invert the accumulator for the direction change
                motor_3.setDirection(st_pre.mot[MOTOR_3].direction);
            }
            motor_3.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_3, st_pre.mot[MOTOR_3].step_sign);
        } else {
            st_run.mot[MOTOR_3].substep_increment_increment = 0;
            motor_3.motionStopped();
        }
        ACCUMULATE_ENCODER(MOTOR_3);
#endif
#if (MOTORS >= 4)
        if ((st_run.mot[MOTOR_4].substep_increment = st_pre.mot[MOTOR_4].substep_increment) != 0) {
            st_run.mot[MOTOR_4].substep_increment_increment = st_pre.mot[MOTOR_4].substep_increment_increment;
            if (st_pre.mot[MOTOR_4].direction != st_pre.mot[MOTOR_4].prev_direction) {
                st_pre.mot[MOTOR_4].prev_direction = st_pre.mot[MOTOR_4].direction;
                st_run.mot[MOTOR_4].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_4].substep_accumulator); // invert the accumulator for the direction change
                motor_4.setDirection(st_pre.mot[MOTOR_4].direction);
            }
            motor_4.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_4, st_pre.mot[MOTOR_4].step_sign);
        } else {
            st_run.mot[MOTOR_4].substep_increment_increment = 0;
            motor_4.motionStopped();
        }
        ACCUMULATE_ENCODER(MOTOR_4);
#endif
#if (MOTORS >= 5)
        if ((st_run.mot[MOTOR_5].substep_increment = st_pre.mot[MOTOR_5].substep_increment) != 0) {
            st_run.mot[MOTOR_5].substep_increment_increment = st_pre.mot[MOTOR_5].substep_increment_increment;
            if (st_pre.mot[MOTOR_5].direction != st_pre.mot[MOTOR_5].prev_direction) {
                st_pre.mot[MOTOR_5].prev_direction = st_pre.mot[MOTOR_5].direction;
                st_run.mot[MOTOR_5].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_5].substep_accumulator); // invert the accumulator for the direction change
                motor_5.setDirection(st_pre.mot[MOTOR_5].direction);
            }
            motor_5.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_5, st_pre.mot[MOTOR_5].step_sign);
        } else {
            st_run.mot[MOTOR_5].substep_increment_increment = 0;
            motor_5.motionStopped();
        }
        ACCUMULATE_ENCODER(MOTOR_5);
#endif
#if (MOTORS >= 6)
        if ((st_run.mot[MOTOR_6].substep_increment = st_pre.mot[MOTOR_6].substep_increment) != 0) {
            st_run.mot[MOTOR_6].substep_increment_increment = st_pre.mot[MOTOR_6].substep_increment_increment;
            if (st_pre.mot[MOTOR_6].direction != st_pre.mot[MOTOR_6].prev_direction) {
                st_pre.mot[MOTOR_6].prev_direction = st_pre.mot[MOTOR_6].direction;
                st_run.mot[MOTOR_6].substep_accumulator = -(DDA_SUBSTEPS + st_run.mot[MOTOR_6].substep_accumulator); // invert the accumulator for the direction change
                motor_6.setDirection(st_pre.mot[MOTOR_6].direction);
            }
            motor_6.enable();
            SET_ENCODER_STEP_SIGN(MOTOR_6, st_pre.mot[MOTOR_6].step_sign);
        } else {
            st_run.mot[MOTOR_6].substep_increment_increment = 0;
            motor_6.motionStopped();
        }
        ACCUMULATE_ENCODER(MOTOR_6);
#endif

        //**** do this last ****

        st_run.dda_ticks_downcount = st_pre.dda_ticks;

    // handle dwells and commands
    } else if (st_pre.block_type == BLOCK_TYPE_DWELL) {
        st_run.dwell_ticks_downcount = st_pre.dwell_ticks;

        // We now use SysTick events to handle dwells
        SysTickTimer.registerEvent(&dwell_systick_event);

    // handle synchronous commands
    } else if (st_pre.block_type == BLOCK_TYPE_COMMAND) {
        mp_runtime_command(st_pre.bf);

    } // else null - which is okay in many cases

    // all other cases drop to here (e.g. Null moves after Mcodes skip to here)
    st_pre.block_type = BLOCK_TYPE_NULL;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_EXEC;    // we are done with the prep buffer - flip the flag back
    st_request_exec_move();                             // exec and prep next move
}

/***********************************************************************************
 * st_prep_line() - Prepare the next move for the loader
 *
 *  This function does the math on the next pulse segment and gets it ready for
 *  the loader. It deals with all the DDA optimizations and timer setups so that
 *  loading can be performed as rapidly as possible. It works in joint space
 *  (motors) and it works in steps, not length units. All args are provided as
 *  floats and converted to their appropriate integer types for the loader.
 *
 * Args:
 *    - travel_steps[] are signed relative motion in steps for each motor. Steps are
 *      floats that typically have fractional values (fractional steps). The sign
 *      indicates direction. Motors that are not in the move should be 0 steps on input.
 *
 *    - following_error[] is a vector of measured errors to the step count. Used for correction.
 *
 *    - segment_time - how many minutes the segment should run. If timing is not
 *      100% accurate this will affect the move velocity, but not the distance traveled.
 *
 * NOTE:  Many of the expressions are sensitive to casting and execution order to avoid long-term
 *        accuracy errors due to floating point round off. One earlier failed attempt was:
 *          dda_ticks_X_substeps = (int32_t)((microseconds/1000000) * f_dda * dda_substeps);
 */

stat_t st_prep_line(const float start_velocity, const float end_velocity, const float travel_steps[], const float following_error[], const float segment_time)
{
    // trap assertion failures and other conditions that would prevent queuing the line
    if (st_pre.buffer_state != PREP_BUFFER_OWNED_BY_EXEC) {     // never supposed to happen
        return (cm_panic(STAT_INTERNAL_ERROR, "st_prep_line() prep sync error"));
    } else if (isinf(segment_time)) {                           // never supposed to happen
        return (cm_panic(STAT_PREP_LINE_MOVE_TIME_IS_INFINITE, "st_prep_line()"));
    } else if (isnan(segment_time)) {                           // never supposed to happen
        return (cm_panic(STAT_PREP_LINE_MOVE_TIME_IS_NAN, "st_prep_line()"));
    }
    // setup segment parameters
    // - dda_ticks is the integer number of DDA clock ticks needed to play out the segment
    // - ticks_X_substeps is the maximum depth of the DDA accumulator (as a negative number)

    //st_pre.dda_period = _f_to_period(FREQUENCY_DDA);                // FYI: this is a constant
    st_pre.dda_ticks = (int32_t)(segment_time * 60 * FREQUENCY_DDA);// NB: converts minutes to seconds

    // setup motor parameters
    // this is explained later
    double t_v0_v1 = (double)st_pre.dda_ticks * (start_velocity + end_velocity);

    float correction_steps;
    for (uint8_t motor=0; motor<MOTORS; motor++) {          // remind us that this is motors, not axes
        float steps = travel_steps[motor];

        // Skip this motor if there are no new steps. Leave all other values intact.
        if (fp_ZERO(steps)) {
            st_pre.mot[motor].substep_increment = 0;        // substep increment also acts as a motor flag
            continue;
        }

        // Setup the direction, compensating for polarity.
        // Set the step_sign which is used by the stepper ISR to accumulate step position

        if (steps >= 0) {                    // positive direction
            st_pre.mot[motor].direction = DIRECTION_CW ^ st_cfg.mot[motor].polarity;
            st_pre.mot[motor].step_sign = 1;
        } else {
            st_pre.mot[motor].direction = DIRECTION_CCW ^ st_cfg.mot[motor].polarity;
            st_pre.mot[motor].step_sign = -1;
        }


        // 'Nudge' correction strategy. Inject a single, scaled correction value then hold off
        // NOTE: This clause can be commented out to test for numerical accuracy and accumulating errors
        if ((--st_pre.mot[motor].correction_holdoff < 0) &&
            (std::abs(following_error[motor]) > STEP_CORRECTION_THRESHOLD)) {

            st_pre.mot[motor].correction_holdoff = STEP_CORRECTION_HOLDOFF;
            correction_steps = following_error[motor] * STEP_CORRECTION_FACTOR;

            if (correction_steps > 0) {
                correction_steps = std::min(std::min(correction_steps, std::abs(steps)), STEP_CORRECTION_MAX);
            } else {
                correction_steps = std::max(std::max(correction_steps, -std::abs(steps)), -STEP_CORRECTION_MAX);
            }
            st_pre.mot[motor].corrected_steps += correction_steps;
            steps -= correction_steps;
        }

        // Compute substep increment. The accumulator must be *exactly* the incoming
        // fractional steps times the substep multiplier or positional drift will occur.
        // Rounding is performed to eliminate a negative bias in the uint32 conversion
        // that results in long-term negative drift. (std::abs/round order doesn't matter)

        //  t is ticks duration of the move
        //  T is time duration of the move in minutes
        //  f is dda frequency, ticks/sec
        //  s is steps for the move
        //  n is unknown scale factor
        //       whatever the kinematics end up with to convert mm to steps for this motor and segment
        //  v_0 and v_1 are the start and end velocity (in mm/min)
        //
        //  t = T 60 f
        //  Note: conversion from minutes to seconds cancels out in n
        //  n = (s/(T 60))/(((v_0/60)+(v_1/60))/2) = (2 s)/(T(v_0 + v_1))
        //
        //  Needed is steps/tick
        //  1/m_0 = (n (v_0/60))/f
        //  1/m_1 = (n (v_1/60))/f
        //
        //  Substitute n:
        //  1/m_0 = ((2 s)/(T(v_0 + v_1)) (v_0/60))/f = (s v_0)/(T 30 f (v_0 + v_1)) = (2 s v_0)/(t (v_0 + v_1))
        //  1/m_1 = ((2 s)/(T(v_0 + v_1)) (v_1/60))/f = (s v_1)/(T 30 f (v_0 + v_1)) = (2 s v_1)/(t (v_0 + v_1))
        //  d = (1/m_1-1/m_0)/(t-1) = (2 s (v_1 - v_0))/((t - 1) t (v_0 + v_1))
        //  Some common terms:
        //  a = t (v_0 + v_1)
        //  b = 2 s
        //  c = 1/m_0 = (b v_0)/a
        // option 1:
        //  d = ((b v_1)/a - c)/(t-1)
        // option 2:
        //  d = (b (v_1 - v_0))/((t-1) a)

        double s_double = std::abs(steps * 2.0);

        // 1/m_0 = (2 s v_0)/(t (v_0 + v_1))
        st_pre.mot[motor].substep_increment = round(((s_double * start_velocity)/(t_v0_v1)) * (double)DDA_SUBSTEPS);
        // option 1:
        //  d = ((b v_1)/a - c)/(t-1)
        // option 2:
        //  d = (b (v_1 - v_0))/((t-1) a)
        st_pre.mot[motor].substep_increment_increment = round(((s_double*(end_velocity-start_velocity))/(((double)st_pre.dda_ticks-1.0)*t_v0_v1)) * (double)DDA_SUBSTEPS);
    }
    st_pre.block_type = BLOCK_TYPE_ALINE;
    st_pre.bf = nullptr;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER;    // signal that prep buffer is ready

    return (STAT_OK);
}

// same as previous function, except it takes a different start and end velocity per motor
stat_t st_prep_line(const float start_velocities[], const float end_velocities[], const float travel_steps[], const float following_error[], const float segment_time)
{
    // TODO refactor out common parts of the two st_prep_line functions

    // trap assertion failures and other conditions that would prevent queuing the line
    if (st_pre.buffer_state != PREP_BUFFER_OWNED_BY_EXEC) {     // never supposed to happen
        return (cm_panic(STAT_INTERNAL_ERROR, "st_prep_line() prep sync error"));
    } else if (isinf(segment_time)) {                           // never supposed to happen
        return (cm_panic(STAT_PREP_LINE_MOVE_TIME_IS_INFINITE, "st_prep_line()"));
    } else if (isnan(segment_time)) {                           // never supposed to happen
        return (cm_panic(STAT_PREP_LINE_MOVE_TIME_IS_NAN, "st_prep_line()"));
//    } else if (segment_time < EPSILON) {
//        return (STAT_MINIMUM_TIME_MOVE);
    }
    // setup segment parameters
    // - dda_ticks is the integer number of DDA clock ticks needed to play out the segment
    // - ticks_X_substeps is the maximum depth of the DDA accumulator (as a negative number)

    //st_pre.dda_period = _f_to_period(FREQUENCY_DDA);                // FYI: this is a constant
    st_pre.dda_ticks = (int32_t)(segment_time * 60 * FREQUENCY_DDA);// NB: converts minutes to seconds

    float correction_steps;
    for (uint8_t motor=0; motor<MOTORS; motor++) {          // remind us that this is motors, not axes
        float steps = travel_steps[motor];

        // setup motor parameters
        double t_v0_v1 = (double)st_pre.dda_ticks * (start_velocities[motor] + end_velocities[motor]);

        // Skip this motor if there are no new steps. Leave all other values intact.
        if (fp_ZERO(steps)) {
            st_pre.mot[motor].substep_increment = 0;        // substep increment also acts as a motor flag
            continue;
        }

        // Setup the direction, compensating for polarity.
        // Set the step_sign which is used by the stepper ISR to accumulate step position

        if (steps >= 0) {                    // positive direction
            st_pre.mot[motor].direction = DIRECTION_CW ^ st_cfg.mot[motor].polarity;
            st_pre.mot[motor].step_sign = 1;
        } else {
            st_pre.mot[motor].direction = DIRECTION_CCW ^ st_cfg.mot[motor].polarity;
            st_pre.mot[motor].step_sign = -1;
        }


        // 'Nudge' correction strategy. Inject a single, scaled correction value then hold off
        // NOTE: This clause can be commented out to test for numerical accuracy and accumulating errors
        if ((--st_pre.mot[motor].correction_holdoff < 0) &&
            (std::abs(following_error[motor]) > STEP_CORRECTION_THRESHOLD)) {

            st_pre.mot[motor].correction_holdoff = STEP_CORRECTION_HOLDOFF;
            correction_steps = following_error[motor] * STEP_CORRECTION_FACTOR;

            if (correction_steps > 0) {
                correction_steps = std::min(std::min(correction_steps, std::abs(steps)), STEP_CORRECTION_MAX);
            } else {
                correction_steps = std::max(std::max(correction_steps, -std::abs(steps)), -STEP_CORRECTION_MAX);
            }
            st_pre.mot[motor].corrected_steps += correction_steps;
            steps -= correction_steps;
        }

        // All math is explained in the previous function

        double s_double = std::abs(steps * 2.0);
        st_pre.mot[motor].substep_increment = round(((s_double * start_velocities[motor])/(t_v0_v1)) * (double)DDA_SUBSTEPS);
        st_pre.mot[motor].substep_increment_increment = round(((s_double*(end_velocities[motor]-start_velocities[motor]))/(((double)st_pre.dda_ticks-1.0)*t_v0_v1)) * (double)DDA_SUBSTEPS);
    }
    st_pre.block_type = BLOCK_TYPE_ALINE;
    st_pre.bf = nullptr;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER;    // signal that prep buffer is ready
    return (STAT_OK);
}
/*
 * st_prep_null() - Keeps the loader happy. Otherwise performs no action
 */

void st_prep_null()
{
    st_pre.block_type = BLOCK_TYPE_NULL;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_EXEC;    // signal that prep buffer is empty
}

/*
 * st_prep_command() - Stage command to execution
 */

void st_prep_command(void *bf)
{
    st_pre.block_type = BLOCK_TYPE_COMMAND;
    st_pre.bf = (mpBuf_t *)bf;
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER;    // signal that prep buffer is ready
}

/*
 * st_prep_dwell()      - Add a dwell to the move buffer
 */

void st_prep_dwell(float milliseconds)
{
    st_pre.block_type = BLOCK_TYPE_DWELL;
    // we need dwell_ticks to be at least 1
    st_pre.dwell_ticks = std::max((uint32_t)((milliseconds/1000.0) * FREQUENCY_DWELL), (uint32_t)1UL);
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER;    // signal that prep buffer is ready
}

/*
 * st_prep_out_of_band_dwell()
 *
 * Add a dwell to the loader without going through the planner buffers.
 * Only usable while exec isn't running, e.g. in feedhold or stopped states.
 * Otherwise it is skipped.
 */

void st_prep_out_of_band_dwell(float milliseconds)
{
    st_prep_dwell(milliseconds);
    st_pre.buffer_state = PREP_BUFFER_OWNED_BY_LOADER;    // signal that prep buffer is ready
    st_request_load_move();
}

/*
 * _set_hw_microsteps() - set microsteps in hardware
 */

static void _set_hw_microsteps(const uint8_t motor, const uint16_t microsteps)
{
    if (motor >= MOTORS) { return; }

    Motors[motor]->setMicrosteps(microsteps);
}

/***********************************************************************************
 * CONFIGURATION AND INTERFACE FUNCTIONS
 * Functions to get and set variables from the cfgArray table
 ***********************************************************************************/

/* HELPERS
 * _motor() - motor number as an index or -1 if na
 */

static int8_t _motor(const index_t index)
{
    char c = cfgArray[index].token[0];
    return (isdigit(c) ? c-0x31 : -1 ); // 0x30 + 1 offsets motor 1 to == 0
}

/*
 * _set_motor_steps_per_unit() - what it says
 * This function will need to be rethought if microstep morphing is implemented
 */

static float _set_motor_steps_per_unit(nvObj_t *nv)
{
    uint8_t m = _motor(nv->index);
    st_cfg.mot[m].units_per_step = (st_cfg.mot[m].travel_rev * st_cfg.mot[m].step_angle) /
                                   (360 * st_cfg.mot[m].microsteps);

    st_cfg.mot[m].steps_per_unit = 1/st_cfg.mot[m].units_per_step;

    kn_config_changed();

    return (st_cfg.mot[m].steps_per_unit);
}

/* PER-MOTOR FUNCTIONS
 *
 * st_get_ma() - get motor axis mapping
 * st_set_ma() - set motor axis mapping
 * st_get_sa() - get motor step angle
 * st_set_sa() - set motor step angle
 * st_get_tr() - get travel per motor revolution
 * st_set_tr() - set travel per motor revolution
 * st_get_mi() - get motor microsteps
 * st_set_mi() - set motor microsteps
 *
 * st_set_pm() - set motor power mode
 * st_get_pm() - get motor power mode
 * st_set_pl() - set motor power level
 * st_set_pi() - set motor idle power level
 */

/*
 * st_get_ma() - get motor axis mapping
 *
 *  Legacy axis numbers are     XYZABC    for axis 0-5
 *  External axis numbers are   XYZABCUVW for axis 0-8
 *  Internal axis numbers are   XYZUVWABC for axis 0-8 (for various code reasons)
 *
 *  This function retrieves an internal axis number and remaps it to an external axis number
 */
stat_t st_get_ma(nvObj_t *nv)
{
#if (AXES == 9)
    uint8_t remap_axis[9] = { 0,1,2,6,7,8,3,4,5 };
#else
    uint8_t remap_axis[6] = { 0,1,2,3,4,5 };
#endif
    ritorno(get_integer(nv, st_cfg.mot[_motor(nv->index)].motor_map));
    nv->value_int = remap_axis[nv->value_int];
    return(STAT_OK);
}

/*
 * st_set_ma() - set motor axis mapping
 *
 *  Legacy axis numbers are     XYZABC    for axis 0-5
 *  External axis numbers are   XYZABCUVW for axis 0-8
 *  Internal axis numbers are   XYZUVWABC for axis 0-8 (for various code reasons)
 *
 *  This function accepts an external axis number and remaps it to an external axis number,
 *  writes the internal axis number and returns the external number in the JSON response.
 */
stat_t st_set_ma(nvObj_t *nv)
{
    if (nv->value_int < 0) {
        nv->valuetype = TYPE_NULL;
        return (STAT_INPUT_LESS_THAN_MIN_VALUE);
    }
    if (nv->value_int > AXES) {
        nv->valuetype = TYPE_NULL;
        return (STAT_INPUT_EXCEEDS_MAX_VALUE);
    }
    uint8_t external_axis = nv->value_int;
#if (AXES == 9)
    uint8_t remap_axis[9] = { 0,1,2,6,7,8,3,4,5 };
#else
    uint8_t remap_axis[6] = { 0,1,2,3,4,5 };
#endif
    nv->value_int = remap_axis[nv->value_int];
    ritorno(set_integer(nv, st_cfg.mot[_motor(nv->index)].motor_map, 0, AXES));
    nv->value_int = external_axis;
    kn_config_changed();
    return(STAT_OK);
}

// step angle
stat_t st_get_sa(nvObj_t *nv) { return(get_float(nv, st_cfg.mot[_motor(nv->index)].step_angle)); }
stat_t st_set_sa(nvObj_t *nv)
{
    ritorno(set_float_range(nv, st_cfg.mot[_motor(nv->index)].step_angle, 0.001, 360));
    _set_motor_steps_per_unit(nv);
    return(STAT_OK);
}

// travel per revolution
stat_t st_get_tr(nvObj_t *nv) { return(get_float(nv, st_cfg.mot[_motor(nv->index)].travel_rev)); }
stat_t st_set_tr(nvObj_t *nv)
{
    ritorno(set_float_range(nv, st_cfg.mot[_motor(nv->index)].travel_rev, 0.0001, 1000000));
    _set_motor_steps_per_unit(nv);
    return(STAT_OK);
}

// microsteps
stat_t st_get_mi(nvObj_t *nv) { return(get_integer(nv, st_cfg.mot[_motor(nv->index)].microsteps)); }
stat_t st_set_mi(nvObj_t *nv)
{
    if (nv->value_int <= 0) {
        nv->valuetype = TYPE_NULL;
        return (STAT_INPUT_LESS_THAN_MIN_VALUE);
    }

    uint16_t mi = nv->value_int;
    if ((mi != 1) && (mi != 2) && (mi != 4) && (mi != 8) && (mi != 16) && (mi != 32)) {
        nv_add_conditional_message((const char *)"*** WARNING *** Setting non-standard microstep value");
    }
    // set it anyway, even if it's unsupported
    ritorno(set_uint32(nv, st_cfg.mot[_motor(nv->index)].microsteps, 1, 256));
    _set_motor_steps_per_unit(nv);
    _set_hw_microsteps(_motor(nv->index), nv->value_int);
    return (STAT_OK);
}

// motor steps per unit (direct)
stat_t st_get_su(nvObj_t *nv)
{
    return(get_float(nv, st_cfg.mot[_motor(nv->index)].steps_per_unit));
}

stat_t st_set_su(nvObj_t *nv)
{
    // Don't set a zero or negative value - just calculate based on sa, tr, and mi
    // This way, if STEPS_PER_UNIT is set to 0 it is unused and we get the computed value
    if(nv->value_flt <= 0) {
        nv->value_flt = _set_motor_steps_per_unit(nv);
        return(STAT_OK);
    }

    // Do unit conversion here because it's a reciprocal value (rather than process_incoming_float())
    if (cm_get_units_mode(MODEL) == INCHES) {
        if (cm_get_axis_type(nv) == AXIS_TYPE_LINEAR) {
            nv->value_flt *= INCHES_PER_MM;
        }
    }
    uint8_t m = _motor(nv->index);
    st_cfg.mot[m].steps_per_unit = nv->value_flt;
    st_cfg.mot[m].units_per_step = 1.0/st_cfg.mot[m].steps_per_unit;

    // Scale TR so all the other values make sense
    // You could scale any one of the other values, but TR makes the most sense
    st_cfg.mot[m].travel_rev = (360.0 * st_cfg.mot[m].microsteps) /
                               (st_cfg.mot[m].steps_per_unit * st_cfg.mot[m].step_angle);
    return(STAT_OK);
}

// polarity
stat_t st_get_po(nvObj_t *nv) { return(get_integer(nv, st_cfg.mot[_motor(nv->index)].polarity)); }
stat_t st_set_po(nvObj_t *nv) { return(set_integer(nv, st_cfg.mot[_motor(nv->index)].polarity, 0, 1)); }

// power management mode
stat_t st_get_pm(nvObj_t *nv)
{
    nv->value_int = (float)Motors[_motor(nv->index)]->getPowerMode();
    nv->valuetype = TYPE_INTEGER;
    return (STAT_OK);
}

stat_t st_set_pm(nvObj_t *nv)
{
    // Test the value without setting it, then setPowerMode() now
    // to both set and take effect immediately.
    ritorno(set_integer(nv, (uint8_t &)cs.null, 0, MOTOR_POWER_MODE_MAX_VALUE ));
    Motors[_motor(nv->index)]->setPowerMode((stPowerMode)nv->value_int);
    return (STAT_OK);
}

/*
 * st_get_pl() - get motor power level
 * st_get_pi() - get motor idle power level
 * st_set_pl() - set motor power level
 * st_set_pi() - set motor idle power level
 *
 *  Input value may vary from 0.000 to 1.000 The setting is scaled to allowable PWM range.
 *  This function sets both the scaled and dynamic power levels, and applies the
 *  scaled value to the vref.
 */
stat_t st_get_pl(nvObj_t *nv) { return(get_float(nv, st_cfg.mot[_motor(nv->index)].power_level)); }
stat_t st_get_pi(nvObj_t *nv) { return(get_float(nv, st_cfg.mot[_motor(nv->index)].power_level_idle)); }
stat_t st_set_pl(nvObj_t *nv)
{
    uint8_t m = _motor(nv->index);
    ritorno(set_float_range(nv, st_cfg.mot[m].power_level, 0.0, 1.0));
    st_cfg.mot[m].power_level = nv->value_flt;
    Motors[m]->setPowerLevels(st_cfg.mot[m].power_level, st_cfg.mot[m].power_level_idle);
    return(STAT_OK);
}

stat_t st_set_pi(nvObj_t *nv) {
    uint8_t m = _motor(nv->index);
    ritorno(set_float_range(nv, st_cfg.mot[m].power_level_idle, 0.0, 1.0));
    st_cfg.mot[m].power_level_idle = nv->value_flt;
    Motors[m]->setPowerLevels(st_cfg.mot[m].power_level, st_cfg.mot[m].power_level_idle);
    return (STAT_OK);
}

/*
 * st_get_pwr()	- get current motor power
 *
 *  Returns the current power level of the motor given it's enable/disable state
 *  Returns 0.0 if motor is de-energized or disabled
 *  Can be extended to report idle setback by changing getCurrentPowerLevel()
 */
stat_t st_get_pwr(nvObj_t *nv)
{
    // extract the motor number from the table
    uint8_t c = cfgArray[nv->index].token[3]; // example: pwr1
    int8_t motor = (isdigit(c) ? c-0x31 : -1 ); // 0x30 + 1 offsets motor 1 to == 0
    if (motor < 0 || motor >= MOTORS) { return STAT_INPUT_VALUE_RANGE_ERROR; };

    nv->value_flt = Motors[motor]->getCurrentPowerLevel();
	nv->valuetype = TYPE_FLOAT;
    nv->precision = cfgArray[nv->index].precision;
	return (STAT_OK);
}

stat_t st_set_ep(nvObj_t *nv)            // set motor enable polarity
{
    if (nv->value_int < IO_ACTIVE_LOW) { return (STAT_INPUT_LESS_THAN_MIN_VALUE); }
    if (nv->value_int > IO_ACTIVE_HIGH) { return (STAT_INPUT_EXCEEDS_MAX_VALUE); }

    uint8_t motor = _motor(nv->index);
    if (motor >= MOTORS) { return STAT_INPUT_VALUE_RANGE_ERROR; };

    Motors[motor]->setEnablePolarity((ioPolarity)nv->value_int);
    return (STAT_OK);
}

stat_t st_get_ep(nvObj_t *nv)            // get motor enable polarity
{
    if (nv->value_int < IO_ACTIVE_LOW) { return (STAT_INPUT_LESS_THAN_MIN_VALUE); }
    if (nv->value_int > IO_ACTIVE_HIGH) { return (STAT_INPUT_EXCEEDS_MAX_VALUE); }

    uint8_t motor = _motor(nv->index);
    if (motor >= MOTORS) { return STAT_INPUT_VALUE_RANGE_ERROR; };

    nv->value_int = (float)Motors[motor]->getEnablePolarity();
    nv->valuetype = TYPE_INTEGER;
    return (STAT_OK);
}

stat_t st_set_sp(nvObj_t *nv)            // set motor step polarity
{
    if (nv->value_int < IO_ACTIVE_LOW) { return (STAT_INPUT_LESS_THAN_MIN_VALUE); }
    if (nv->value_int > IO_ACTIVE_HIGH) { return (STAT_INPUT_EXCEEDS_MAX_VALUE); }

    uint8_t motor = _motor(nv->index);
    if (motor >= MOTORS) { return STAT_INPUT_VALUE_RANGE_ERROR; };

    Motors[motor]->setStepPolarity((ioPolarity)nv->value_int);
    return (STAT_OK);
}

stat_t st_get_sp(nvObj_t *nv)            // get motor step polarity
{
    if (nv->value_int < IO_ACTIVE_LOW) { return (STAT_INPUT_LESS_THAN_MIN_VALUE); }
    if (nv->value_int > IO_ACTIVE_HIGH) { return (STAT_INPUT_EXCEEDS_MAX_VALUE); }

    uint8_t motor = _motor(nv->index);
    if (motor >= MOTORS) { return STAT_INPUT_VALUE_RANGE_ERROR; };

    nv->value_int = (float)Motors[motor]->getStepPolarity();
    nv->valuetype = TYPE_INTEGER;
    return (STAT_OK);
}

/* GLOBAL FUNCTIONS (SYSTEM LEVEL)
 *
 * st_get_mt() - get motor timeout in seconds
 * st_set_mt() - set motor timeout in seconds
 * st_set_md() - disable motor power
 * st_set_me() - enable motor power
 * st_set_md() - disable motor power
 * st_get_dw() - get remaining dwell time
 *
 * Calling me or md with NULL will enable or disable all motors
 * Setting a value of 0 will enable or disable all motors
 * Setting a value from 1 to MOTORS will enable or disable that motor only
 */

stat_t st_get_mt(nvObj_t *nv) { return(get_float(nv, st_cfg.motor_power_timeout)); }
stat_t st_set_mt(nvObj_t *nv) {
    ritorno(set_float_range(nv, st_cfg.motor_power_timeout, MOTOR_TIMEOUT_SECONDS_MIN, MOTOR_TIMEOUT_SECONDS_MAX));
    for (uint8_t motor = MOTOR_1; motor < MOTORS; motor++) {
        Motors[motor]->setActivityTimeout(st_cfg.motor_power_timeout*1000.0);
    }
    return (STAT_OK);
}

// Make sure this function is not part of initialization --> f00
// nv->value is seconds of timeout
stat_t st_set_me(nvObj_t *nv)
{
    for (uint8_t motor = MOTOR_1; motor < MOTORS; motor++) {
        Motors[motor]->enableWithTimeout(nv->value_int * 1000.0);   // nv->value is the timeout or 0 for default
    }
    return (STAT_OK);
}

// Make sure this function is not part of initialization --> f00
// nv-value is motor to disable, or 0 for all motors
stat_t st_set_md(nvObj_t *nv)
{
    if (nv->value_int < 0) {
        nv->valuetype = TYPE_NULL;
        return (STAT_INPUT_LESS_THAN_MIN_VALUE);
    }
    if (nv->value_int > MOTORS) {
        nv->valuetype = TYPE_NULL;
        return (STAT_INPUT_EXCEEDS_MAX_VALUE);
    }
    // de-energize all motors
    if ((uint8_t)nv->value_int == 0) {      // 0 means all motors
        for (uint8_t motor = MOTOR_1; motor < MOTORS; motor++) {
            Motors[motor]->disable();
        }
    } else {                            // otherwise it's just one motor
         Motors[(uint8_t)nv->value_int-1]->disable();
    }
    return (STAT_OK);
}

stat_t st_get_dw(nvObj_t *nv)
{
    nv->value_int = st_run.dwell_ticks_downcount;
    nv->valuetype = TYPE_INTEGER;
    return (STAT_OK);
}

/***********************************************************************************
 * TEXT MODE SUPPORT
 * Functions to print variables from the cfgArray table
 ***********************************************************************************/

#ifdef __TEXT_MODE

#ifndef SINGLE_TRANSLATION_BUILD
static const char msg_units0[] = " in";    // used by generic print functions
static const char msg_units1[] = " mm";
static const char msg_units2[] = " deg";
static const char *const msg_units[] = { msg_units0, msg_units1, msg_units2 };
#define DEGREE_INDEX 2
#endif

static const char fmt_me[] = "motors energized\n";
static const char fmt_md[] = "motors de-energized\n";
static const char fmt_mt[] = "[mt]  motor idle timeout%14.2f seconds\n";
static const char fmt_0ma[] = "[%s%s] m%s map to axis%15d [0=X,1=Y,2=Z...]\n";
static const char fmt_0sa[] = "[%s%s] m%s step angle%20.3f%s\n";
static const char fmt_0tr[] = "[%s%s] m%s travel per revolution%10.4f%s\n";
static const char fmt_0mi[] = "[%s%s] m%s microsteps%16d [1,2,4,8,16,32]\n";
static const char fmt_0su[] = "[%s%s] m%s steps per unit %17.5f steps per%s\n";
static const char fmt_0po[] = "[%s%s] m%s polarity%18d [0=normal,1=reverse]\n";
static const char fmt_0ep[] = "[%s%s] m%s enable polarity%11d [0=active HIGH,1=active LOW]\n";
static const char fmt_0sp[] = "[%s%s] m%s step polarity%13d [0=active HIGH,1=active LOW]\n";
static const char fmt_0pm[] = "[%s%s] m%s power management%10d [0=disabled,1=always on,2=in cycle,3=when moving,4=reduced when idle]\n";
static const char fmt_0pl[] = "[%s%s] m%s motor power level%13.3f [0.000=minimum, 1.000=maximum]\n";
static const char fmt_0pi[] = "[%s%s] m%s motor idle power level%13.3f [0.000=minimum, 1.000=maximum]\n";
static const char fmt_pwr[] = "[%s%s] Motor %c power level:%12.3f\n";

void st_print_me(nvObj_t *nv) { text_print(nv, fmt_me);}    // TYPE_NULL - message only
void st_print_md(nvObj_t *nv) { text_print(nv, fmt_md);}    // TYPE_NULL - message only
void st_print_mt(nvObj_t *nv) { text_print(nv, fmt_mt);}    // TYPE_FLOAT

static void _print_motor_int(nvObj_t *nv, const char *format)
{
    sprintf(cs.out_buf, format, nv->group, nv->token, nv->group, (int)nv->value_int);
    xio_writeline(cs.out_buf);
}

static void _print_motor_flt(nvObj_t *nv, const char *format)
{
    sprintf(cs.out_buf, format, nv->group, nv->token, nv->group, nv->value_flt);
    xio_writeline(cs.out_buf);
}

static void _print_motor_flt_units(nvObj_t *nv, const char *format, uint8_t units)
{
    sprintf(cs.out_buf, format, nv->group, nv->token, nv->group, nv->value_flt, GET_TEXT_ITEM(msg_units, units));
    xio_writeline(cs.out_buf);
}

static void _print_motor_pwr(nvObj_t *nv, const char *format)
{
    sprintf(cs.out_buf, format, nv->group, nv->token, nv->token[0], nv->value_flt);
    xio_writeline(cs.out_buf);
}

void st_print_ma(nvObj_t *nv) { _print_motor_int(nv, fmt_0ma);}
void st_print_sa(nvObj_t *nv) { _print_motor_flt_units(nv, fmt_0sa, DEGREE_INDEX);}
void st_print_tr(nvObj_t *nv) { _print_motor_flt_units(nv, fmt_0tr, cm_get_units_mode(MODEL));}
void st_print_mi(nvObj_t *nv) { _print_motor_int(nv, fmt_0mi);}
void st_print_su(nvObj_t *nv) { _print_motor_flt_units(nv, fmt_0su, cm_get_units_mode(MODEL));}
void st_print_po(nvObj_t *nv) { _print_motor_int(nv, fmt_0po);}
void st_print_ep(nvObj_t *nv) { _print_motor_int(nv, fmt_0ep);}
void st_print_sp(nvObj_t *nv) { _print_motor_int(nv, fmt_0sp);}
void st_print_pm(nvObj_t *nv) { _print_motor_int(nv, fmt_0pm);}
void st_print_pl(nvObj_t *nv) { _print_motor_flt(nv, fmt_0pl);}
void st_print_pi(nvObj_t *nv) { _print_motor_flt(nv, fmt_0pi);}
void st_print_pwr(nvObj_t *nv){ _print_motor_pwr(nv, fmt_pwr);}

#endif // __TEXT_MODE
