/*
 * stepper.h - stepper motor interface
 * This file is part of TinyG project
 *
 * Copyright (c) 2010 - 2013 Alden S. Hart, Jr.
 * Copyright (c) 2013 Robert Giseburt
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
 *	Coordinated motion (line drawing) is performed using a classic Bresenham DDA. 
 *	A number of additional steps are taken to optimize interpolation and pulse train
 *	accuracy and minimize pulse jitter.
 *
 *	  - The DDA accepts and processes fractional motor steps. Steps are passed to the 
 *		move queue as floats, and do not need to be integer values. The DDA implements 
 *		fractional steps and interpolation by extending the counter range downward using 
 *		a fixed-point binary number whose precision is set using DDA_SUBSTEPS. 
 *
 *    - The DDA is not used as a 'ramp' for acceleration management. Accel is computed 
 *		as 3rd order (controlled jerk) equations that generate accel/decel segments to 
 *		the DDA in much the same way arc drawing is approximated. 
 *
 *    - Constant Rate DDA clock: The DDA runs at a constant, maximum rate for every 
 *		segment regardless of actual step rate required. This means that the DDA clock 
 *		is not tuned to the step rate (or a multiple) of the major axis, as many other 
 *		DDAs do. Running the DDA flat out might appear to be "wasteful", but it ensures 
 *		that the best aliasing results are achieved. 
 *
 *		The observation is that TinyG is a hard real-time system in which every clock cycle 
 *		is knowable and can be accounted for. So if the system is capable of sustaining
 *		max pulse rate for the fastest move, it's capable of sustaining this rate for any
 *		move. So just run it flat out and get the best pulse resolution for all moves. 
 *		If we were running from batteries we might not be so cavalier about this.
 *
 *    - Pulse phasing is preserved between segments if possible. This makes for smoother
 *		motion, particularly at very low speeds and short segment lengths (avoids pulse 
 *		jitter). Phase continuity is achieved by simply not resetting the DDA counters 
 *		across segments. In some cases the differences between timer values across 
 *		segments are too large for this to work, and you risk motor stalls due to pulse 
 *		starvation. These cases are detected and the counters are reset to prevent stalling.
 *
 *    - Pulse phasing is also helped by minimizing the time spent loading the next move 
 *		segment. To this end as much as possible about that move is pre-computed during 
 *		move execution. Also, all moves are loaded from the interrupt level, avoiding 
 *		the need for mutual exclusion locking or volatiles (which slow things down).
 */
/**** Line planning and execution ****
 *
 *	Move planning, execution and pulse generation takes place at 3 levels:
 *
 *	Move planning occurs in the main-loop. The canonical machine calls the planner to 
 *	generate lines, arcs, dwells and synchronous stop/starts. The planner module generates 
 *	blocks (bf's) that hold parameters for lines and the other move types. The blocks 
 *	are backplanned to join lines, and to take dwells and stops into account. ("plan" stage).
 *
 *	Arc movement is planned above the above the line planner. The arc planner generates 
 *	short lines that are passed to the line planner.
 *
 *	Move execution and load prep takes place at the LOW interrupt level. Move execution 
 *	generates the next acceleration, cruise, or deceleration segment for planned lines, 
 *	or just transfers parameters needed for dwells and stops. This layer also prepares 
 *	moves for loading by pre-calculating the values needed by the DDA, and converting the 
 *	executed move into parameters that can be directly loaded into the steppers ("exec" 
 *	and "prep" stages).
 *
 *	Pulse train generation takes place at the HI interrupt level. The stepper DDA fires 
 *	timer interrupts that generate the stepper pulses. This level also transfers new 
 *	stepper parameters once each pulse train ("segment") is complete ("load" and "run" stages). 
 */
/* 	What happens when the pulse generator is done with the current pulse train (segment) 
 *	is a multi-stage "pull" queue that looks like this:
 *
 *	As long as the steppers are running the sequence of events is:
 *
 *	  - The stepper interrupt (HI) runs the DDA to generate a pulse train for the 
 *		current move. This runs for the length of the pulse train currently executing 
 *		- the "segment", usually 5ms worth of pulses
 *
 *	  - When the current segment is finished the stepper interrupt LOADs the next segment 
 *		from the prep buffer, reloads the timers, and starts the next segment. At the end 
 *		of the load the stepper interrupt routine requests an "exec" of the next move in 
 *		order to prepare for the next load operation. It does this by calling the exec 
 *		using a software interrupt (actually a timer, since that's all we've got).
 *
 *	  - As a result of the above, the EXEC handler fires at the LO interrupt level. It 
 *		computes the next accel/decel segment for the current move (i.e. the move in the 
 *		planner's runtime buffer) by calling back to the exec routine in planner.c. 
 *		Or it gets and runs the next buffer in the planning queue - depending on the 
 *		move_type and state. 
 *
 *	  - Once the segment has been computed the exec handler finshes up by running the 
 *		PREP routine in stepper.c. This computes the DDA values and gets the segment 
 *		into the prep buffer - and ready for the next LOAD operation.
 *
 *	  - The main loop runs in background to receive gcode blocks, parse them, and send 
 *		them to the planner in order to keep the planner queue full so that when the 
 *		planner's runtime buffer completes the next move (a gcode block or perhaps an 
 *		arc segment) is ready to run.
 *
 *	If the steppers are not running the above is similar, except that the exec is 
 *	invoked from the main loop by the software interrupt, and the stepper load is 
 *	invoked from the exec by another software interrupt.
 */
/*	Control flow can be a bit confusing. This is a typical sequence for planning 
 *	executing, and running an acceleration planned line:
 *
 *	 1  planner.mp_aline() is called, which populates a planning buffer (bf) 
 *		and back-plans any pre-existing buffers.
 *
 *	 2  When a new buffer is added _mp_queue_write_buffer() tries to invoke
 *	    execution of the move by calling stepper.st_request_exec_move(). 
 *
 *	 3a If the steppers are running this request is ignored.
 *	 3b If the steppers are not running this will set a timer to cause an 
 *		EXEC "software interrupt" that will ultimately call st_exec_move().
 *
 *   4  At this point a call to _exec_move() is made, either by the 
 *		software interrupt from 3b, or once the steppers finish running 
 *		the current segment and have loaded the next segment. In either 
 *		case the call is initated via the EXEC software interrupt which 
 *		causes _exec_move() to run at the MEDium interupt level.
 *		 
 *	 5	_exec_move() calls back to planner.mp_exec_move() which generates 
 *		the next segment using the mr singleton.
 *
 *	 6	When this operation is complete mp_exec_move() calls the appropriate
 *		PREP routine in stepper.c to derive the stepper parameters that will 
 *		be needed to run the move - in this example st_prep_line().
 *
 *	 7	st_prep_line() generates the timer and DDA values and stages these into 
 *		the prep structure (sp) - ready for loading into the stepper runtime struct
 *
 *	 8	stepper.st_prep_line() returns back to planner.mp_exec_move(), which 
 *		frees the planning buffer (bf) back to the planner buffer pool if the 
 *		move is complete. This is done by calling _mp_request_finalize_run_buffer()
 *
 *	 9	At this point the MED interrupt is complete, but the planning buffer has 
 *		not actually been returned to the pool yet. The buffer will be returned
 *		by the main-loop prior to testing for an available write buffer in order
 *		to receive the next Gcode block. This handoff prevents possible data 
 *		conflicts between the interrupt and main loop.
 *
 *	10	The final step in the sequence is _load_move() requesting the next 
 *		segment to be executed and prepared by calling st_request_exec() 
 *		- control goes back to step 4.
 *
 *	Note: For this to work you have to be really careful about what structures
 *	are modified at what level, and use volatiles where necessary.
 */
/* Partial steps and phase angle compensation
 *
 *	The DDA accepts partial steps as input. Fractional steps are managed by the 
 *	sub-step value as explained elsewhere. The fraction initially loaded into 
 *	the DDA and the remainder left at the end of a move (the "residual") can
 *	be thought of as a phase angle value for the DDA accumulation. Each 360
 *	degrees of phase angle results in a step being generated. 
 */
#ifndef STEPPER_H_ONCE
#define STEPPER_H_ONCE

/*********************************
 * Stepper configs and constants *
 *********************************/
//See hardware.h for platform specific stepper definitions

// Currently there is no distinction between IDLE and OFF (DEENERGIZED)
// In the future IDLE will be powered at a low, torque-maintaining current

enum motorPowerState {				// used w/start and stop flags to sequence motor power
	MOTOR_OFF = 0,					// motor is stopped and deenergized
	MOTOR_IDLE,						// motor is stopped and may be partially energized for torque maintenance
	MOTOR_TIME_IDLE_TIMEOUT,		// run idle timeout
	MOTOR_START_IDLE_TIMEOUT,		// transitional state to start idle timers
	MOTOR_STOPPED,					// motor is stopped and fully energized
	MOTOR_RUNNING					// motor is running (and fully energized)
};

enum cmStepperPowerMode {
	MOTOR_ENERGIZED_DURING_CYCLE=0,	// motor is fully powered during cycles
	MOTOR_IDLE_WHEN_STOPPED,		// idle motor shortly after it's stopped - even in cycle
	MOTOR_POWER_REDUCED_WHEN_IDLE,	// enable Vref current reduction (not implemented yet)
	DYNAMIC_MOTOR_POWER				// adjust motor current with velocity (not implemented yet)
};

enum prepBufferState {
	PREP_BUFFER_OWNED_BY_LOADER = 0,// staging buffer is ready for load
	PREP_BUFFER_OWNED_BY_EXEC		// staging buffer is being loaded
};

// Stepper power management settings
// Min/Max timeouts allowed for motor disable. Allow for inertial stop; must be non-zero
#define IDLE_TIMEOUT_SECONDS_MIN 	(float)0.1		// seconds !!! SHOULD NEVER BE ZERO !!!
#define IDLE_TIMEOUT_SECONDS_MAX	(float)4294967	// (4294967295/1000) -- for conversion to uint32_t
#define IDLE_TIMEOUT_SECONDS 		(float)0.1		// seconds in DISABLE_AXIS_WHEN_IDLE mode

/* DDA substepping
 * 	DDA_SUBSTEPS sets the amount of fractional precision for substepping.
 *	Substepping is kind of like microsteps done in software to make
 *	interpolation more accurate.
 *
 *	Set to 1 to disable, but don't do this or you will lose a lot of accuracy.
 */
#define DDA_SUBSTEPS 100000		// 100,000 accumulates substeps to 6 decimal places

/* Accumulator resets
 * 	You want to reset the DDA accumulators if the new ticks value is way less 
 *	than previous value, but otherwise you should leave the accumulators alone.
 *	Preserving the accumulator value from the previous segment aligns pulse 
 *	phasing between segments. However, if the new accumulator is going to be 
 *	much less than the old one you must reset it or risk motor stalls.
 */
#define ACCUMULATOR_RESET_FACTOR 2	// amount counter range can safely change

/*
 * Stepper control structures
 *
 *	There are 4 sets of structures involved in this operation;
 *
 *	data structure:						static to:		runs at:
 *	  mpBuffer planning buffers (bf)	  planner.c		  main loop
 *	  mrRuntimeSingleton (mr)			  planner.c		  MED ISR
 *	  stPrepSingleton (sp)				  stepper.c		  MED ISR
 *	  stRunSingleton (st)				  stepper.c		  HI ISR
 *  
 *	Care has been taken to isolate actions on these structures to the 
 *	execution level in which they run and to use the minimum number of 
 *	volatiles in these structures. This allows the compiler to optimize
 *	the stepper inner-loops better.
 */

typedef struct cfgMotor {			// per-motor configs
	uint8_t	motor_map;				// map motor to axis
  	uint8_t microsteps;				// microsteps to apply for each axis (ex: 8)
	uint8_t polarity;				// 0=normal polarity, 1=reverse motor direction
 	uint8_t power_mode;				// See stepper.h for enum
	float step_angle;				// degrees per whole step (ex: 1.8)
	float travel_rev;				// mm or deg of travel per motor revolution
	float steps_per_unit;			// steps (usteps)/mm or deg of travel
} cfgMotor_t;

typedef struct stConfig {			// stepper configs
	float motor_idle_timeout;		// seconds before setting motors to idle current (currently this is OFF)
	cfgMotor_t m[MOTORS];			// settings for motors 1-4
} stConfig_t;

// Runtime structure. Used exclusively by step generation ISR (HI)

typedef struct stRunMotor { 		// one per controlled motor
	int32_t phase_increment;		// total steps in axis times substeps factor
	int32_t phase_accumulator;		// DDA phase angle accumulator for axis
	uint8_t power_state;			// state machine for managing motor power
	uint32_t power_systick;			// sys_tick for next state transition
	uint32_t power_level;			// power level for this segment (FUTURE)
	uint8_t step_count_diagnostic;	// step count diagnostic
} stRunMotor_t;

typedef struct stRunSingleton {		// Stepper static values and axis parameters
	uint16_t magic_start;			// magic number to test memory integrity	
	int32_t dda_ticks_downcount;	// tick down-counter (unscaled)
	int32_t dda_ticks_X_substeps;	// ticks multiplied by scaling factor
	stRunMotor_t m[MOTORS];			// runtime motor structures
} stRunSingleton_t;

// Prep-time structure. Used by exec/prep ISR (MED) and read-only during load
// Must be careful about volatiles in this one

typedef struct stPrepMotor {
 	uint32_t phase_increment; 		// total steps in axis times substep factor
	int8_t dir;						// direction
} stPrepMotor_t;

typedef struct stPrepSingleton {
	uint16_t magic_start;			// magic number to test memory integrity	
	uint8_t move_type;				// move type
	volatile uint8_t exec_state;	// move execution state 
	volatile uint8_t reset_flag;	// TRUE if accumulator should be reset
	uint32_t prev_ticks;			// tick count from previous move
	uint16_t dda_period;			// DDA or dwell clock period setting
	uint32_t dda_ticks;				// DDA or dwell ticks for the move
	uint32_t dda_ticks_X_substeps;	// DDA ticks scaled by substep factor
//	float segment_velocity;			// record segment velocity for diagnostics
	stPrepMotor_t m[MOTORS];		// per-motor structs
} stPrepSingleton_t;

extern stConfig_t st;

/**** FUNCTION PROTOTYPES ****/

void stepper_init(void);
uint8_t stepper_isbusy(void);
stat_t st_assertions(void);

void st_energize_motors(void);
void st_deenergize_motors(void);
void st_set_motor_power(const uint8_t motor);
stat_t st_motor_power_callback(void);

void st_request_exec_move(void);
void st_prep_null(void);
void st_prep_dwell(float microseconds);
stat_t st_prep_line(float steps[], float microseconds);

stat_t st_set_sa(cmdObj_t *cmd);
stat_t st_set_tr(cmdObj_t *cmd);
stat_t st_set_mi(cmdObj_t *cmd);
stat_t st_set_pm(cmdObj_t *cmd);
stat_t st_set_mt(cmdObj_t *cmd);
stat_t st_set_md(cmdObj_t *cmd);
stat_t st_set_me(cmdObj_t *cmd);

#ifdef __TEXT_MODE

	void st_print_mt(cmdObj_t *cmd);
	void st_print_me(cmdObj_t *cmd);
	void st_print_md(cmdObj_t *cmd);
	void st_print_ma(cmdObj_t *cmd);
	void st_print_sa(cmdObj_t *cmd);
	void st_print_tr(cmdObj_t *cmd);
	void st_print_mi(cmdObj_t *cmd);
	void st_print_po(cmdObj_t *cmd);
	void st_print_pm(cmdObj_t *cmd);

#else

	#define st_print_mt tx_print_stub
	#define st_print_me tx_print_stub
	#define st_print_md tx_print_stub
	#define st_print_ma tx_print_stub
	#define st_print_sa tx_print_stub
	#define st_print_tr tx_print_stub
	#define st_print_mi tx_print_stub
	#define st_print_po tx_print_stub
	#define st_print_pm tx_print_stub

#endif // __TEXT_MODE

#endif	// End of include guard: STEPPER_H_ONCE
