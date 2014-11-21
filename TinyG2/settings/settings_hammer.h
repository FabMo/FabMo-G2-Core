/*
 * settings_hammer.h - machine profile for hammer demo
 * Part of TinyG project
 *
 * Copyright (c) 2013 - 2014 Alden S. Hart Jr.
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
/***********************************************************************/
/**** Default profile for hammer demo **********************************/
/***********************************************************************/
// NOTE: Non-machine specific systems settings have been moved to settings.h
// These may be overridden using undefs

// ***> NOTE: The init message must be a single line with no CRs or LFs 
#define INIT_MESSAGE "Initializing configs to HAMMER settings"

#define VELOCITY_MAX			1000			// mm/min (converts to degrees / min)
#define JERK_MAX 				40				// 40 million mm/(min^3)

#define ROTARY_VELOCITY_MAX		28800
#define ROTARY_JERK_MAX			1400			// set for 1.2 lb hammer

#define JUNCTION_DEVIATION		0.05		// default value, in mm
#define JUNCTION_ACCELERATION	100000		// centripetal acceleration around corners

// **** settings.h overrides ****

#undef MOTOR_POWER_TIMEOUT
#define MOTOR_POWER_TIMEOUT		4						// seconds to maintain motor at full power before idling

// *** motor settings ***

#define M1_MOTOR_MAP 			AXIS_X					// 1ma
#define M1_STEP_ANGLE 			1.8						// 1sa
#define M1_TRAVEL_PER_REV		20						// 1tr
#define M1_MICROSTEPS			8						// 1mi		1,2,4,8
#define M1_POLARITY				0						// 1po		0=normal, 1=reversed
#define M1_POWER_MODE			MOTOR_POWERED_IN_CYCLE	// 1pm		standard
#define M1_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M2_MOTOR_MAP	 		AXIS_Y
#define M2_STEP_ANGLE			1.8
#define M2_TRAVEL_PER_REV		20
#define M2_MICROSTEPS			8
#define M2_POLARITY				0
#define M2_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M2_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M3_MOTOR_MAP			AXIS_Z
#define M3_STEP_ANGLE			1.8
#define M3_TRAVEL_PER_REV		20
#define M3_MICROSTEPS			8
#define M3_POLARITY				0
#define M3_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M3_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M4_MOTOR_MAP			AXIS_A
#define M4_STEP_ANGLE			1.8
#define M4_TRAVEL_PER_REV		360						// degrees moved per motor rev
#define M4_MICROSTEPS			8
#define M4_POLARITY				0
#define M4_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M4_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M5_MOTOR_MAP			AXIS_B
#define M5_STEP_ANGLE			1.8
#define M5_TRAVEL_PER_REV		360
#define M5_MICROSTEPS			8
#define M5_POLARITY				0
#define M5_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M5_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M6_MOTOR_MAP			AXIS_C
#define M6_STEP_ANGLE			1.8
#define M6_TRAVEL_PER_REV		360
#define M6_MICROSTEPS			8
#define M6_POLARITY				0
#define M6_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M6_POWER_LEVEL			MOTOR_POWER_LEVEL

// *** axis settings ***

#define X_AXIS_MODE 			AXIS_STANDARD		// xam		see canonical_machine.h cmAxisMode for valid values
#define X_VELOCITY_MAX 			VELOCITY_MAX 		// xvm		G0 max velocity in mm/min
#define X_FEEDRATE_MAX 			X_VELOCITY_MAX		// xfr 		G1 max feed rate in mm/min
#define X_TRAVEL_MIN			0					// xtn		monimum travel for soft limits
#define X_TRAVEL_MAX 			150					// xtm		travel between switches or crashes
#define X_JERK_MAX 				JERK_MAX			// xjm
#define X_JUNCTION_DEVIATION 	JUNCTION_DEVIATION	// xjd
#define X_SWITCH_MODE_MIN 		SW_MODE_HOMING		// xsn		SW_MODE_DISABLED, SW_MODE_HOMING, SW_MODE_LIMIT, SW_MODE_HOMING_LIMIT
#define X_SWITCH_MODE_MAX 		SW_MODE_DISABLED	// xsx		SW_MODE_DISABLED, SW_MODE_HOMING, SW_MODE_LIMIT, SW_MODE_HOMING_LIMIT
#define X_SWITCH_TYPE_MIN       SW_TYPE_NORMALLY_OPEN       // rsn    SW_TYPE_NORMALLY_OPEN, SW_TYPE_NORMALLY_CLOSED
#define X_SWITCH_TYPE_MAX       SW_TYPE_NORMALLY_OPEN       // rsx    SW_TYPE_NORMALLY_OPEN, SW_TYPE_NORMALLY_CLOSED
#define X_SEARCH_VELOCITY 		500					// xsv		move in negative direction
#define X_LATCH_VELOCITY 		100					// xlv		mm/min
#define X_LATCH_BACKOFF 		2					// xlb		mm
#define X_ZERO_BACKOFF 			0.25				// xzb		mm
#define X_JERK_HOMING			X_JERK_MAX			// xjh

#define Y_AXIS_MODE 			AXIS_STANDARD
#define Y_VELOCITY_MAX 			VELOCITY_MAX
#define Y_FEEDRATE_MAX 			Y_VELOCITY_MAX
#define Y_TRAVEL_MIN			0
#define Y_TRAVEL_MAX 			150
#define Y_JERK_MAX 				JERK_MAX
#define Y_JUNCTION_DEVIATION 	JUNCTION_DEVIATION
#define Y_SWITCH_MODE_MIN 		SW_MODE_HOMING
#define Y_SWITCH_MODE_MAX 		SW_MODE_DISABLED
#define Y_SWITCH_TYPE_MIN       SW_TYPE_NORMALLY_OPEN
#define Y_SWITCH_TYPE_MAX       SW_TYPE_NORMALLY_OPEN
#define Y_SEARCH_VELOCITY 		500
#define Y_LATCH_VELOCITY 		100
#define Y_LATCH_BACKOFF 		2
#define Y_ZERO_BACKOFF 			0.25
#define Y_JERK_HOMING			Y_JERK_MAX

#define Z_AXIS_MODE 			AXIS_STANDARD
#define Z_VELOCITY_MAX 			VELOCITY_MAX
#define Z_FEEDRATE_MAX 			Z_VELOCITY_MAX
#define Z_TRAVEL_MIN			0
#define Z_TRAVEL_MAX 			75
#define Z_JERK_MAX 				JERK_MAX
#define Z_JUNCTION_DEVIATION 	JUNCTION_DEVIATION
#define Z_SWITCH_MODE_MIN 		SW_MODE_DISABLED
#define Z_SWITCH_MODE_MAX 		SW_MODE_HOMING
#define Z_SWITCH_TYPE_MIN       SW_TYPE_NORMALLY_OPEN
#define Z_SWITCH_TYPE_MAX       SW_TYPE_NORMALLY_OPEN
#define Z_SEARCH_VELOCITY 		400
#define Z_LATCH_VELOCITY 		100
#define Z_LATCH_BACKOFF 		2
#define Z_ZERO_BACKOFF 			0.25
#define Z_JERK_HOMING			Z_JERK_MAX

#define A_AXIS_MODE 			AXIS_STANDARD
#define A_VELOCITY_MAX 			ROTARY_VELOCITY_MAX
#define A_FEEDRATE_MAX 			ROTARY_VELOCITY_MAX
#define A_TRAVEL_MIN 			-1
#define A_TRAVEL_MAX 			-1										// -1 means infinite, no limit
#define A_JERK_MAX 				ROTARY_JERK_MAX
#define A_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define A_RADIUS 				(M4_TRAVEL_PER_REV/(2*3.14159628))
#define A_SWITCH_MODE_MIN 		SW_MODE_HOMING
#define A_SWITCH_MODE_MAX 		SW_MODE_DISABLED
#define A_SWITCH_TYPE_MIN       SW_TYPE_NORMALLY_OPEN
#define A_SWITCH_TYPE_MAX       SW_TYPE_NORMALLY_OPEN
#define A_SEARCH_VELOCITY 		600
#define A_LATCH_VELOCITY 		100
#define A_LATCH_BACKOFF 		5		// degrees
#define A_ZERO_BACKOFF 			2
#define A_JERK_HOMING			A_JERK_MAX

//#define A_VELOCITY_MAX 		((VELOCITY_MAX/M4_TRAVEL_PER_REV)*360) // set to the same speed as X axis
//#define A_VELOCITY_MAX 		172800
//#define A_JERK_MAX 			(JERK_MAX*(360/M4_TRAVEL_PER_REV))
//#define A_JERK_MAX 			5760
//#define A_RADIUS 				0.19894368

#define B_AXIS_MODE 			AXIS_STANDARD
#define B_VELOCITY_MAX 			ROTARY_VELOCITY_MAX
#define B_FEEDRATE_MAX 			ROTARY_VELOCITY_MAX
#define B_TRAVEL_MAX 			-1
#define B_TRAVEL_MIN			-1
#define B_JERK_MAX 				ROTARY_JERK_MAX
#define B_JUNCTION_DEVIATION 	JUNCTION_DEVIATION
#define B_RADIUS 				1

#define C_AXIS_MODE 			AXIS_STANDARD
#define C_VELOCITY_MAX 			ROTARY_VELOCITY_MAX
#define C_FEEDRATE_MAX 			ROTARY_VELOCITY_MAX
#define C_TRAVEL_MAX 			-1
#define C_TRAVEL_MIN			-1
#define C_JERK_MAX 				ROTARY_JERK_MAX
#define C_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define C_RADIUS				1

// *** DEFAULT COORDINATE SYSTEM OFFSETS ***

#define G54_X_OFFSET 0			// G54 is traditionally set to all zeros
#define G54_Y_OFFSET 0
#define G54_Z_OFFSET 0
#define G54_A_OFFSET 0
#define G54_B_OFFSET 0
#define G54_C_OFFSET 0

#define G55_X_OFFSET (X_TRAVEL_MAX/2)	// set to middle of table
#define G55_Y_OFFSET (Y_TRAVEL_MAX/2)
#define G55_Z_OFFSET 0
#define G55_A_OFFSET 0
#define G55_B_OFFSET 0
#define G55_C_OFFSET 0

#define G56_X_OFFSET 0
#define G56_Y_OFFSET 0
#define G56_Z_OFFSET 0
#define G56_A_OFFSET 0
#define G56_B_OFFSET 0
#define G56_C_OFFSET 0

#define G57_X_OFFSET 0
#define G57_Y_OFFSET 0
#define G57_Z_OFFSET 0
#define G57_A_OFFSET 0
#define G57_B_OFFSET 0
#define G57_C_OFFSET 0

#define G58_X_OFFSET 0
#define G58_Y_OFFSET 0
#define G58_Z_OFFSET 0
#define G58_A_OFFSET 0
#define G58_B_OFFSET 0
#define G58_C_OFFSET 0

#define G59_X_OFFSET 0
#define G59_Y_OFFSET 0
#define G59_Z_OFFSET 0
#define G59_A_OFFSET 0
#define G59_B_OFFSET 0
#define G59_C_OFFSET 0
