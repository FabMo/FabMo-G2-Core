/*
 * stepper.h - stepper motor interface
 * This file is part of g2core project
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
/*
 * --- Stepper Operation Overview ---
 *
 *  Coordinated motion (line drawing) is performed using a classic Bresenham DDA.
 *  Additional steps are taken to optimize interpolation and pulse train timing
 *  accuracy to minimize pulse jitter and produce very smooth motion and surface
 *  finish.
 *
 *  - The DDA is not used as a ramp for acceleration management. Acceleration is computed
 *    upstream in the motion planner as 6th order (linear pop) equations. These
 *    generate accel/decel *segments* that are passed to the DDA for step output.
 *
 *  - The DDA accepts and processes fractional motor steps as floating point numbers
 *    from the planner. Steps do not need to be whole numbers and are not expected to be.
 *    The step values are converted to integer by multiplying by an integer value
 *    (DDA_SUBSTEPS) to roughly preserve the precision of the floating point number
 *    in the 32 bit int. Rounding is performed to avoid a truncation bias.
 *
 *  - Constant Rate DDA clock: The DDA runs at a constant, maximum rate for every
 *    segment regardless of actual step rate required. This means that the DDA clock
 *    is not tuned to the step rate (or a multiple) of the major axis, as is typical
 *    for DDAs. Running the DDA flat out might appear to be "wasteful", but it ensures
 *    that the best aliasing results are achieved, and is part of maintaining step
 *    accuracy across motion segments.
 *
 *    The observation is that g2core is a hard real-time system in which every clock cycle
 *    is knowable and can be accounted for. So if the system is capable of sustaining
 *    max pulse rate for the fastest move, it's capable of sustaining this rate for any
 *    move. So we just run it flat out and get the best pulse resolution for all moves.
 *    If we were running from batteries or otherwise cared about the energy budget we
 *    might not be so cavalier about this.
 *
 *    On most ARM implementation the DDA clock runs at 400 KHz, but it's bi-phasic so the effective
 *    step rate is 200 KHz. This leaves 2.5 uSec between pulse timer (DDA) interrupts.
 *    This consumes roughly 15 - 20% of the 84 MHz CPU clock for pulsing 6 motors.
 *
 *  - Pulse timing is also helped by minimizing the time spent loading the next move
 *    segment. The time budget for the load is less than the time remaining before the
 *    next DDA clock tick. This means that the load must take < 5 uSec (Arm) or the
 *    time between pulses will stretch out when changing segments. This does not affect
 *    positional accuracy but can affect jitter and smoothness. To this end as much
 *    as possible about that move is pre-computed during move execution (prep cycles).
 *    Also, all moves are loaded from the DDA interrupt level (HI), avoiding the need
 *    for mutual exclusion locking or volatiles (which slow things down).
 */
/*
 **** Move generation overview and timing illustration ****
 *
 *  This ASCII art illustrates a 4 segment move to show stepper sequencing timing.
 *
 *  LOAD/STEP (~5000uSec)          [L1][segment1][L2][segment2][L3][segment3][L4][segment4][Lb1]
 *  PREP (100 uSec)            [P1]       [P2]          [P3]          [P4]          [Pb1]
 *  EXEC (400 uSec)         [EXEC1]    [EXEC2]       [EXEC3]       [EXEC4]       [EXECb1]
 *  PLAN (<4ms)  [planmoveA][plan move B][plan move C][plan move D][plan move E] etc.
 *
 *  The move begins with the planner planning move A [planmoveA]. When this is done the
 *  computations for the first segment of move A's S curve are performed by the planner
 *  runtime, EXEC1. The runtime computes the number of segments and the segment-by-segment
 *  accelerations and decelerations for the move. Each call to EXEC generates the values
 *  for the next segment to be run. Once the move is running EXEC is executed as a
 *  callback from the step loader.
 *
 *  When the runtime calculations are done EXEC calls the segment preparation function [P1].
 *  PREP turns the EXEC results into values needed for the loader and does some encoder work.
 *  The combined exec and prep takes about 400 uSec.
 *
 *  PREP takes care of heavy numerics and other cycle-intensive operations so the step loader
 *  L1 can run as fast as possible. The time budget for LOAD is about 5 uSec. In the diagram,
 *  when P1 is done segment 1 is loaded into the stepper runtime [L1]
 *
 *  Once the segment is loaded it will pulse out steps for the duration of the segment.
 *  Segment timing can vary, but segments are typically between 750 - 1500 microseconds,
 *  making for an average update rate of about 1 KHz.
 *
 *  Now the move is pulsing out segment 1 (at HI interrupt level). Once the L1 loader is
 *  finished it invokes the exec function for the next segment (at LO interrupt level).
 *  [EXEC2] and [P2] compute and prepare the segment 2 for the loader so it can be loaded
 *  as soon as segment 1 is complete [L2]. When move A is done EXEC pulls the next move
 *  (moveB) from the planner queue, The process repeats until there are no more segments or moves.
 *
 *  While all this is happening subsequent moves (B, C, and D) are being planned in background.
 *  As long as a move takes less than the total segment times (1ms x N) the timing budget is satisfied
 *
 *  A few things worth noting:
 *  - This scheme uses 2 interrupt levels and background, for 3 levels of execution:
 *    - STEP pulsing and LOADs occur at HI interrupt level
 *    - EXEC and PREP occur at LO interrupt level (leaving MED int level for serial IO)
 *    - move PLANning occurs in background and is managed by the controller
 *
 *  - Because of the way the timing is laid out there is no contention for resources between
 *    the STEP, LOAD, EXEC, and PREP phases. PLANing is similarly isolated. Very few volatiles
 *    or mutexes are needed, which makes the code simpler and faster. You can count on
 *    LOAD, EXEC, PREP and PLAN not stepping on each other's variables.
 */
/**** Line planning and execution (in more detail) ****
 *
 *  Move planning, execution and pulse generation takes place at 3 levels:
 *
 *  Move planning occurs in the main-loop. The canonical machine calls the planner to
 *  generate lines, arcs, dwells, synchronous stop/starts, and any other command that
 *  needs to be synchronized with motion. The planner module generates blocks (bf's)
 *  that hold parameters for lines and the other move types. The blocks are backplanned
 *  to join lines and to take dwells and stops into account. ("plan" stage).
 *
 *  Arc movement is planned above the line planner. The arc planner generates short
 *  lines that are passed to the line planner.
 *
 *  Once lines are planned the must be broken up into "segments" of about 1 millisecond
 *  to be run. These segments are how S curves are generated. This is the job of the move
 *  runtime (aka. exec or mr).
 *
 *  Move execution and load prep takes place at the LOW interrupt level. Move execution
 *  generates the next acceleration, cruise, or deceleration segment for planned lines,
 *  or just transfers parameters needed for dwells and stops. This layer also prepares
 *  segments for loading by pre-calculating the values needed by the DDA and converting
 *  the segment into parameters that can be directly loaded into the steppers ("exec"
 *  and "prep" stages).
 *
 *  Pulse train generation takes place at the HI interrupt level. The stepper DDA fires
 *  timer interrupts that generate the stepper pulses. This level also transfers new
 *  stepper parameters once each pulse train ("segment") is complete ("load" and "run" stages).
 */
/* (This section has on older explanation of the sequencing
 *  that is somewhat redundant with the one provided above)
 *
 * What happens when the pulse generator is done with the current pulse train (segment)
 *  is a multi-stage "pull" queue that looks like this:
 *
 *  As long as the steppers are running the sequence of events is:
 *
 *  - The stepper interrupt (HI) runs the DDA to generate a pulse train for the
 *    current move. This runs for the length of the pulse train currently executing
 *    the "segment", usually ~1ms worth of pulses
 *
 *  - When the current segment is finished the stepper interrupt LOADs the next segment
 *    from the prep buffer, reloads the timers, and starts the next segment. At the end
 *    of the load the stepper interrupt routine requests an "exec" of the next move in
 *    order to prepare for the next load operation. It does this by calling the exec
 *    using a software interrupt (actually a timer, since that's all we've got).
 *
 *    - As a result of the above, the EXEC handler fires at the LO interrupt level. It
 *    computes the next accel/decel or cruise (body) segment for the current move
 *    (i.e. the move in the planner's runtime buffer) by calling back to the exec
 *    routine in planner.c. If there are no more segments to run for the move the
 *    exec first gets the next buffer in the planning queue and begins execution.
 *
 *    In some cases the next "move" is not actually a move, but a dwell, stop, IO
 *    operation (e.g. M5). In this case it executes the requested operation, and may
 *    attempt to get the next buffer from the planner when its done.
 *
 *  - Once the segment has been computed the exec handler finishes up by running the
 *    PREP routine in stepper.cpp. This computes the DDA values and gets the segment
 *    into the prep buffer - and ready for the next LOAD operation.
 *
 *  - The main loop runs in background to receive gcode blocks, parse them, and send
 *    them to the planner in order to keep the planner queue full so that when the
 *    planner's runtime buffer completes the next move (a gcode block or perhaps an
 *    arc segment) is ready to run.
 *
 *  If the steppers are not running the above is similar, except that the exec is
 *  invoked from the main loop by the software interrupt, and the stepper load is
 *  invoked from the exec by another software interrupt.
 */
/* MORE DETAILS OF THE CONTROL FLOW
 *
 *  Control flow can be a bit confusing. This is a typical sequence for planning
 *  executing, and running an acceleration planned line:
 *
 *   1  planner.mp_aline() is called, which populates a planning buffer (bf)
 *    and back-plans any pre-existing buffers.
 *
 *   2  When a new buffer is added _mp_queue_write_buffer() tries to invoke
 *      execution of the move by calling stepper.st_request_exec_move().
 *
 *   3a If the steppers are running this request is ignored.
 *   3b If the steppers are not running this will set a timer to cause an
 *      EXEC "software interrupt" that will ultimately call st_exec_move().
 *
 *   4  At this point a call to _exec_move() is made, either by the
 *      software interrupt from 3b, or once the steppers finish running
 *      the current segment and have loaded the next segment. In either
 *      case the call is initated via the EXEC software interrupt which
 *      causes _exec_move() to run at the MEDium interupt level.
 *
 *   5  _exec_move() calls back to planner.mp_exec_move() which generates
 *      the next segment using the mr singleton.
 *
 *   6  When this operation is complete mp_exec_move() calls the appropriate
 *      PREP routine in stepper.c to derive the stepper parameters that will
 *      be needed to run the move - in this example st_prep_line().
 *
 *   7  st_prep_line() generates the timer and DDA values and stages these into
 *      the prep structure (sp) - ready for loading into the stepper runtime struct
 *
 *   8  stepper.st_prep_line() returns back to planner.mp_exec_move(), which
 *      frees the planning buffer (bf) back to the planner buffer pool if the
 *      move is complete. This is done by calling _mp_request_finalize_run_buffer()
 *
 *   9  At this point the MED interrupt is complete, but the planning buffer has
 *      not actually been returned to the pool yet. The buffer will be returned
 *      by the main-loop prior to testing for an available write buffer in order
 *      to receive the next Gcode block. This handoff prevents possible data
 *      conflicts between the interrupt and main loop.
 *
 *  10  The final step in the sequence is _load_move() requesting the next
 *      segment to be executed and prepared by calling st_request_exec()
 *      - control goes back to step 4.
 *
 *  Note: For this to work you have to be really careful about what structures
 *  are modified at what level, and use volatiles where necessary.
 */
/* Partial steps and phase angle compensation
 *
 *  The DDA accepts partial steps as input. Fractional steps are managed by the
 *  sub-step value as explained elsewhere. The fraction initially loaded into
 *  the DDA and the remainder left at the end of a move (the "residual") can
 *  be thought of as a phase angle value for the DDA accumulation. Each 360
 *  degrees of phase angle results in a step being generated.
 */

#ifndef STEPPER_H_ONCE
#define STEPPER_H_ONCE

#include "g2core.h"
#include "report.h"

#include "MotateUtilities.h" // for HOT_DATA and HOT_FUNC

#include "planner.h"    // planner.h must precede stepper.h for moveType typedef

#include "gpio.h" // for ioPolarity

// Note: "board_stepper.h" is inlcluded at the end of this file

/*********************************
 * Stepper configs and constants *
 *********************************/
//See hardware.h for platform specific stepper definitions

enum prepBufferState {
    PREP_BUFFER_OWNED_BY_LOADER = 0,    // staging buffer is ready for load
    PREP_BUFFER_OWNED_BY_EXEC           // staging buffer is being loaded
};

enum stPowerMode {
    MOTOR_DISABLED = 0,                 // motor enable is deactivated
    MOTOR_ALWAYS_POWERED,               // motor is always powered while machine is ON
    MOTOR_POWERED_IN_CYCLE,             // motor fully powered during cycles, de-powered out of cycle
    MOTOR_POWERED_ONLY_WHEN_MOVING,     // motor only powered while moving - idles shortly after it's stopped - even in cycle
    MOTOR_POWER_REDUCED_WHEN_IDLE       // enable Vref current reduction for idle
};
#define MOTOR_POWER_MODE_MAX_VALUE    MOTOR_POWER_REDUCED_WHEN_IDLE

// Min/Max timeouts allowed for motor disable. Allow for inertial stop; must be non-zero
#define MOTOR_TIMEOUT_SECONDS_MIN   (float)0.1      // seconds !!! SHOULD NEVER BE ZERO !!!
#define MOTOR_TIMEOUT_SECONDS_MAX   (float)4294967  // (4294967295/1000) -- for conversion to uint32_t
                                                    // 1 dog year (7 weeks)

// Step generation constants
#define STEP_INITIAL_DIRECTION        DIRECTION_CW

/* DDA substepping
 *
 *  DDA Substepping is a fixed.point scheme to increase the resolution of the DDA pulse generation
 *  while still using integer math (as opposed to floating point). Improving the accuracy of the DDA
 *  results in more precise pulse timing and therefore less pulse jitter and smoother motor operation.
 *
 *  The DDA accumulator is an int32_t, so the accumulator has the number range of about 2.1 billion.
 *  The DDA_SUBSTEPS is used to multiply the step count for a segment to maximally use this number range.
 *  DDA_SUBSTEPS can be computed for a given DDA clock rate and segment time not to exceed the available
 *  number range. Variables are:
 *
 *    MAX_LONG == 2^31, maximum signed long (depth of accumulator. NB: accumulator values are negative)
 *    FREQUENCY_DDA == DDA clock rate in Hz.
 *    NOM_SEGMENT_TIME == upper bound of segment time in minutes
 *    0.90 == a safety factor used to reduce the result from theoretical maximum ////##th I can't find where this is used ???
 *
 */
 
 #define DDA_SUBSTEPS (2147483600L)
 #define DDA_HALF_SUBSTEPS (DDA_SUBSTEPS/2)


//  Step correction settings ... ////##th OBSOLETE (removed)

/*
 * Stepper control structures
 *
 *  There are 5 main structures involved in stepper operations;
 *
 *  data structure:                   found in:      runs primarily at:
 *    mpBuf_t planning buffers (bf)    planner.c       main loop
 *    mrRuntimeSingleton (mr)           planner.c      MED ISR
 *    stConfig (st_cfg)                 stepper.c      write=bkgd, read=ISRs
 *    stPrepSingleton (st_pre)          stepper.c      MED ISR
 *    stRunSingleton (st_run)           tepper.c       HI ISR
 *
 *  Care has been taken to isolate actions on these structures to the execution level
 *  in which they run and to use the minimum number of volatiles in these structures.
 *  This allows the compiler to optimize the stepper inner-loops better.
 */

// Motor config structure

typedef struct cfgMotor {                   // per-motor configs
    // public
    uint8_t  motor_map;                     // map motor to axis
    uint32_t microsteps;                    // microsteps to apply for each axis (ex: 8)
    uint8_t  polarity;                      // 0=normal polarity, 1=reverse motor direction
    float power_level;                      // set 0.000 to 1.000 for PWM vref setting
    float power_level_idle;                 // set 0.000 to 1.000 for PWM vref idle setting
    float step_angle;                       // degrees per whole step (ex: 1.8)
    float travel_rev;                       // mm or deg of travel per motor revolution
    float steps_per_unit;                   // microsteps per mm (or degree) of travel
    float units_per_step;                   // mm or degrees of travel per microstep
} cfgMotor_t;

typedef struct stConfig {                   // stepper configs
    float motor_power_timeout;              // seconds before setting motors to idle current (currently this is OFF)
    cfgMotor_t mot[MOTORS];                 // settings for motors 1-N
} stConfig_t;

// Motor runtime structure. Used exclusively by step generation ISR (HI)

typedef struct stRunMotor {                 // one per controlled motor
    int32_t substep_increment;              // partial steps to increment substep_accumulator per tick
    int32_t substep_increment_increment;    // partial steps to increment substep_increment per tick
    int32_t substep_accumulator;            // DDA phase angle accumulator
    bool motor_flag;                        // true if motor is participating in this move
    bool start_new_block;                   // Used in stepping runtime and prep
    uint32_t power_systick;                 // sys_tick for next motor power state transition
    float power_level_dynamic;              // power level for this segment of idle
} stRunMotor_t;

typedef struct stRunSingleton {             // Stepper static values and axis parameters
    magic_t magic_start;                    // magic number to test memory integrity
    uint32_t dda_ticks_downcount;           // dda tick down-counter (unscaled)
    uint32_t dwell_ticks_downcount;         // dwell tick down-counter (unscaled)
    stRunMotor_t mot[MOTORS];               // runtime motor structures
    magic_t magic_end;
} stRunSingleton_t;

// Motor prep structure. Used by exec/prep ISR (MED) and read-only during load
// Must be careful about volatiles in this one

typedef struct stPrepMotor {
    int32_t substep_increment;              // partial steps to increment substep_accumulator per tick
    int32_t substep_increment_increment;    // partial steps to increment substep_increment per tick
    bool motor_flag;                        // true if motor is participating in this move

////## Block Initialization Marker          // Used to set initial SUSBSTEP_HALF_DDA in a block to make moves symetrical
    bool start_new_block;                   // Used in stepping runtime and prep
    
    // direction and direction change
    uint8_t direction;                      // travel direction corrected for polarity (CW==0. CCW==1)
    uint8_t prev_direction;                 // travel direction from previous segment run for this motor
    int8_t step_sign;                       // set to +1 or -1 for encoders

    // following error correction
    int32_t correction_holdoff;             // count down segments between corrections
    float corrected_steps;                  // accumulated correction steps for the cycle (for diagnostic display only)

    // accumulator phase correction
    float prev_segment_time;                // segment time from previous segment run for this motor
    float accumulator_correction;           // factor for adjusting accumulator between segments
    uint8_t accumulator_correction_flag;    // signals accumulator needs correction
} stPrepMotor_t;

typedef struct stPrepSingleton {
    magic_t magic_start;                    // magic number to test memory integrity
    volatile prepBufferState buffer_state;  // prep buffer state - owned by exec or loader
    struct mpBuf_t *bf;                    // static pointer to relevant buffer
    blockType block_type;                   // move type (requires planner.h)

    uint32_t dda_ticks;                     // DDA ticks for the move
    float dda_ticks_holdover;               // partial DDA ticks from previous segment
    uint32_t dwell_ticks;                   // dwell ticks remaining
    stPrepMotor_t mot[MOTORS];              // prep time motor structs
    magic_t magic_end;
} stPrepSingleton_t;

extern stConfig_t st_cfg        HOT_DATA;   // config struct is exposed. The rest are private
extern stPrepSingleton_t st_pre HOT_DATA;   // only used by config_app diagnostics


/**** Stepper (base object) ****/

struct Stepper {
    /* stepper default values */
public:
    // sets default pwm freq for all motor vrefs (commented line below also sets HiZ)
    Stepper()
    {
    };

    /* Functions that handle all motor functions (calls virtuals if needed) */

    virtual void init() // can be overridden
    {
        this->setDirection(STEP_INITIAL_DIRECTION);
    };

    virtual ioPolarity getEnablePolarity() const
    {
        return IO_ACTIVE_LOW; // we have to say something here
    };

    virtual void setEnablePolarity(ioPolarity new_mp)
    {
        // do nothing
    };

    virtual ioPolarity getStepPolarity() const
    {
        return IO_ACTIVE_LOW; // we have to say something here
    };

    virtual void setStepPolarity(ioPolarity new_mp)
    {
        // do nothing
    };

    virtual void setPowerMode(stPowerMode new_pm)
    {
        // do nothing
    };

    virtual stPowerMode getPowerMode()
    {
         return MOTOR_DISABLED;
    };

    virtual float getCurrentPowerLevel()
    {
        // Override to return a proper value
        return (0.0);
    };

    // turn on motor in all cases unless it's disabled
    // this version is called from the loader, and explicitly does NOT have floating point computations
    // HOT - called from the DDA interrupt
    void enable() //HOT_FUNC
    {
        this->_enableImpl();
    };

    virtual void enableWithTimeout(float timeout_ms) // override if wanted
    {
        this->_enableImpl();
    };

    // turn off motor in all cases unless it's permanently enabled
    // HOT - called from the DDA interrupt
    void disable() //HOT_FUNC
    {
        this->_disableImpl();
    };

    // turn off motor is only powered when moving
    // HOT - called from the DDA interrupt
    virtual void motionStopped() {};

    virtual void periodicCheck(bool have_actually_stopped) {}; // can be overridden
    virtual void setActivityTimeout(float idle_milliseconds) {}; // can be overridden

    /* Functions that must be implemented in subclasses */

    virtual bool canStep() { return true; };
    virtual void _enableImpl() { /* must override */ };
    virtual void _disableImpl() { /* must override */ };
    virtual void stepStart() HOT_FUNC { /* must override */ }; // HOT - called from the DDA interrupt
    virtual void stepEnd() HOT_FUNC { /* must override */ };   // HOT - called from the DDA interrupt
    virtual int32_t getStepCount() { return 0; };
    virtual int32_t getStepCountUp() { return 0; };
    virtual int32_t getStepCountDown() { return 0; };
    virtual void resetStepCounts() { /* must override */ };
    virtual void setDirection(uint8_t direction) HOT_FUNC { /* must override */ }; // HOT - called from the DDA interrupt
    virtual void setMicrosteps(const uint16_t microsteps) { /* must override */ };
    virtual void setPowerLevels(float active_pl, float idle_pl) { /* must override */ };
};

/**** ExternalEncoder (base object) ****/

class ExternalEncoder {
   public:
    using callback_t = std::function<void(bool, float)>;
    enum ReturnFormat { ReturnDegrees, ReturnRadians, ReturnFraction };

    virtual void setCallback(std::function<void(bool, float)> &&handler);
    virtual void setCallback(std::function<void(bool, float)> &handler);

    virtual void requestAngleDegrees();
    virtual void requestAngleRadians();
    virtual void requestAngleFraction();

    virtual float getQuadratureFraction();
};

/**** FUNCTION PROTOTYPES ****/

void stepper_init(void);
void stepper_reset(void);
void stepper_init_assertions(void);
stat_t stepper_test_assertions(void);

bool st_runtime_isbusy(void);
stat_t st_clc(nvObj_t *nv);
void st_set_motor_power(const uint8_t motor);
stat_t st_motor_power_callback(void);

void st_request_forward_plan(void) HOT_FUNC;
void st_request_exec_move(void) HOT_FUNC;
void st_request_load_move(void) HOT_FUNC;
void st_prep_null(void);
void st_prep_command(void *bf);        // use a void pointer since we don't know about mpBuf_t yet)
void st_prep_dwell(float milliseconds);
void st_prep_out_of_band_dwell(float milliseconds);
stat_t st_prep_line(const float start_velocity, const float end_velocity, const float travel_steps[], const float following_error[], const float segment_time)  HOT_FUNC;
// NOTE: this version is the same, except it's passed an array of start/end velocities, one pair per motor
stat_t st_prep_line(const float start_velocities[], const float end_velocities[], const float travel_steps[], const float following_error[], const float segment_time)  HOT_FUNC;

stat_t st_get_ma(nvObj_t *nv);
stat_t st_set_ma(nvObj_t *nv);
stat_t st_get_sa(nvObj_t *nv);
stat_t st_set_sa(nvObj_t *nv);
stat_t st_get_tr(nvObj_t *nv);
stat_t st_set_tr(nvObj_t *nv);
stat_t st_get_mi(nvObj_t *nv);
stat_t st_set_mi(nvObj_t *nv);
stat_t st_get_su(nvObj_t *nv);
stat_t st_set_su(nvObj_t *nv);

stat_t st_get_po(nvObj_t *nv);
stat_t st_set_po(nvObj_t *nv);
stat_t st_set_ep(nvObj_t *nv);
stat_t st_get_ep(nvObj_t *nv);
stat_t st_set_sp(nvObj_t *nv);
stat_t st_get_sp(nvObj_t *nv);

stat_t st_get_pm(nvObj_t *nv);
stat_t st_set_pm(nvObj_t *nv);
stat_t st_get_pl(nvObj_t *nv);
stat_t st_get_pi(nvObj_t *nv);
stat_t st_set_pl(nvObj_t *nv);
stat_t st_set_pi(nvObj_t *nv);

stat_t st_get_pwr(nvObj_t *nv);

stat_t st_get_mt(nvObj_t *nv);
stat_t st_set_mt(nvObj_t *nv);
stat_t st_set_md(nvObj_t *nv);
stat_t st_set_me(nvObj_t *nv);
stat_t st_get_dw(nvObj_t *nv);

stat_t st_get_scn(nvObj_t *nv);
stat_t st_get_scu(nvObj_t *nv);
stat_t st_get_scd(nvObj_t *nv);
stat_t st_set_sc(nvObj_t *nv);

#ifdef __TEXT_MODE

    void st_print_ma(nvObj_t *nv);
    void st_print_sa(nvObj_t *nv);
    void st_print_tr(nvObj_t *nv);
    void st_print_mi(nvObj_t *nv);
    void st_print_su(nvObj_t *nv);
    void st_print_po(nvObj_t *nv);
    void st_print_ep(nvObj_t *nv);
    void st_print_sp(nvObj_t *nv);
    void st_print_pm(nvObj_t *nv);
    void st_print_pl(nvObj_t *nv);
    void st_print_pi(nvObj_t *nv);
    void st_print_pwr(nvObj_t *nv);
    void st_print_mt(nvObj_t *nv);
    void st_print_me(nvObj_t *nv);
    void st_print_md(nvObj_t *nv);
    void st_print_scn(nvObj_t *nv);
    void st_print_scu(nvObj_t *nv);
    void st_print_scd(nvObj_t *nv);

#else

    #define st_print_ma tx_print_stub
    #define st_print_sa tx_print_stub
    #define st_print_tr tx_print_stub
    #define st_print_mi tx_print_stub
    #define st_print_su tx_print_stub
    #define st_print_po tx_print_stub
    #define st_print_ep tx_print_stub
    #define st_print_sp tx_print_stub
    #define st_print_pm tx_print_stub
    #define st_print_pl tx_print_stub
    #define st_print_pi tx_print_stub
    #define st_print_pwr tx_print_stub
    #define st_print_mt tx_print_stub
    #define st_print_me tx_print_stub
    #define st_print_md tx_print_stub
    #define st_print_scn tx_print_stub
    #define st_print_scu tx_print_stub
    #define st_print_scd tx_print_stub

#endif // __TEXT_MODE

#include "board_stepper.h"  // include board specific stuff, in particular the Stepper objects

#endif // End of include guard: STEPPER_H_ONCE
