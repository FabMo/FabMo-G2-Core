/*
 * settings_Ultimaker.h - Ultimaker motion demo
 * This file is part of the the TinyG project
 *
 * Copyright (c) 2010 - 2014 Alden S. Hart, Jr.
 * Copyright (c) 2010 - 2014 Robert Giseburt
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
/* Note: The values in this file are the default settings that are loaded
 * 		 into a virgin EEPROM, and can be changed using the config commands.
 *		 After initial load the EEPROM values (or changed values) are used.
 *
 *		 System and hardware settings that you shouldn't need to change 
 *		 are in hardware.h  Application settings that also shouldn't need 
 *		 to be changed are in tinyg.h
 */

/***********************************************************************/
/**** Ultimaker profile ************************************************/
/***********************************************************************/

// ***> NOTE: The init message must be a single line with no CRs or LFs 
#define INIT_MESSAGE "Initializing configs to Ultimaker profile"

#define JUNCTION_DEVIATION		0.5		// default value, in mm
#define JUNCTION_ACCELERATION 	400000		// centripetal acceleration around corners

#ifndef PI
#define PI 3.14159628
#endif

// *** settings.h overrides ***

#undef SWITCH_TYPE
#define SWITCH_TYPE 			SW_TYPE_NORMALLY_OPEN

#undef	COMM_MODE
#define COMM_MODE					JSON_MODE

#undef JSON_VERBOSITY
//#define JSON_VERBOSITY				JV_SILENT		// one of: JV_SILENT, JV_FOOTER, JV_CONFIGS, JV_MESSAGES, JV_LINENUM, JV_VERBOSE
//#define JSON_VERBOSITY				JV_MESSAGES		// one of: JV_SILENT, JV_FOOTER, JV_CONFIGS, JV_MESSAGES, JV_LINENUM, JV_VERBOSE
#define JSON_VERBOSITY				JV_VERBOSE		// one of: JV_SILENT, JV_FOOTER, JV_CONFIGS, JV_MESSAGES, JV_LINENUM, JV_VERBOSE

#undef STATUS_REPORT_VERBOSITY
//#define STATUS_REPORT_VERBOSITY		SR_OFF		// one of: SR_OFF, SR_FILTERED, SR_VERBOSE
//#define STATUS_REPORT_VERBOSITY		SR_FILTERED		// one of: SR_OFF, SR_FILTERED, SR_VERBOSE
#define STATUS_REPORT_VERBOSITY		SR_VERBOSE		// one of: SR_OFF, SR_FILTERED, SR_VERBOSE

#undef STATUS_REPORT_DEFAULTS
//#define STATUS_REPORT_DEFAULTS	"line","posx","posy","posz","vel","_cs1","_es1","_xs1","_fe1","_cs2","_es2","_xs2","_fe2","unit","path","stat"
#define STATUS_REPORT_DEFAULTS	"line","posx","posy","posz","vel","_cs1","_es1","_xs1","_fe1","_cs2","_es2","_xs2","_fe2"

#undef GCODE_DEFAULT_COORD_SYSTEM
#undef GCODE_DEFAULT_UNITS
#undef GCODE_DEFAULT_PLANE
#undef GCODE_DEFAULT_COORD_SYSTEM
#undef GCODE_DEFAULT_PATH_CONTROL
#undef GCODE_DEFAULT_DISTANCE_MODE

#define GCODE_DEFAULT_UNITS			MILLIMETERS		// MILLIMETERS or INCHES
#define GCODE_DEFAULT_PLANE			CANON_PLANE_XY	// CANON_PLANE_XY, CANON_PLANE_XZ, or CANON_PLANE_YZ
#define GCODE_DEFAULT_COORD_SYSTEM	G54				// G54, G55, G56, G57, G58 or G59
#define GCODE_DEFAULT_PATH_CONTROL 	PATH_CONTINUOUS
#define GCODE_DEFAULT_DISTANCE_MODE ABSOLUTE_MODE

// *** motor settings ***

#define M1_MOTOR_MAP 			AXIS_X					// 1ma
#define M1_STEP_ANGLE 			1.8						// 1sa
#define M1_TRAVEL_PER_REV		40.64					// 1tr
#define M1_MICROSTEPS			32						// 1mi		1,2,4,8
#define M1_POLARITY				1						// 1po		0=normal, 1=reversed
#define M1_POWER_MODE			MOTOR_POWERED_IN_CYCLE	// 1pm		standard
#define M1_POWER_LEVEL			0.43		// 1mp

#define M2_MOTOR_MAP	 		AXIS_Y
#define M2_STEP_ANGLE			1.8
#define M2_TRAVEL_PER_REV		40.64
#define M2_MICROSTEPS			32
#define M2_POLARITY				0
#define M2_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M2_POWER_LEVEL			0.43

#define M3_MOTOR_MAP			AXIS_Z
#define M3_STEP_ANGLE			1.8
#define M3_TRAVEL_PER_REV		8.00    // With the HeatBed upgrade
//#define M3_TRAVEL_PER_REV		3.00    // Without the HeatBed upgrade
#define M3_MICROSTEPS			32
#define M3_POLARITY				1
#define M3_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M3_POWER_LEVEL			0.25

#define M4_MOTOR_MAP			AXIS_A
#define M4_STEP_ANGLE			1.8
#define M4_TRAVEL_PER_REV		360			// degrees moved per motor rev
#define M4_MICROSTEPS			32
#define M4_POLARITY				0
#define M4_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M4_POWER_LEVEL			0.40

#define M5_MOTOR_MAP			AXIS_B
#define M5_STEP_ANGLE			1.8
#define M5_TRAVEL_PER_REV		360			// degrees moved per motor rev
#define M5_MICROSTEPS			32
#define M5_POLARITY				0
#define M5_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M5_POWER_LEVEL			MOTOR_POWER_LEVEL

#define M6_MOTOR_MAP			AXIS_C
#define M6_STEP_ANGLE			1.8
#define M6_TRAVEL_PER_REV		360			// degrees moved per motor rev
#define M6_MICROSTEPS			32
#define M6_POLARITY				0
#define M6_POWER_MODE			MOTOR_POWERED_IN_CYCLE
#define M6_POWER_LEVEL			MOTOR_POWER_LEVEL

// *** axis settings ***

#define X_AXIS_MODE 			AXIS_STANDARD		// xam		see canonical_machine.h cmAxisMode for valid values
#define X_VELOCITY_MAX 			18000.0 				// xvm		G0 max velocity in mm/min
#define X_FEEDRATE_MAX 			X_VELOCITY_MAX		// xfr 		G1 max feed rate in mm/min
#define X_TRAVEL_MIN			0					// xtn		minimum travel - used by soft limits and homing
#define X_TRAVEL_MAX 			212					// xtm		travel between switches or crashes
#define X_JERK_MAX 				40000.0				// xjm		yes, that's "100 billion" mm/(min^3)
#define X_JUNCTION_DEVIATION 	JUNCTION_DEVIATION	// xjd
#define X_SWITCH_MODE_MIN		SW_MODE_HOMING		// xsn		SW_MODE_DISABLED, SW_MODE_HOMING, SW_MODE_HOMING_LIMIT, SW_MODE_LIMIT
#define X_SWITCH_MODE_MAX		SW_MODE_LIMIT		// xsx		SW_MODE_DISABLED, SW_MODE_HOMING, SW_MODE_HOMING_LIMIT, SW_MODE_LIMIT
#define X_SEARCH_VELOCITY 		3000				// xsv		move in negative direction
#define X_LATCH_VELOCITY 		200					// xlv		mm/min
#define X_LATCH_BACKOFF 		10					// xlb		mm
#define X_ZERO_BACKOFF 			3					// xzb		mm
#define X_JERK_HOMING			X_JERK_MAX			// xjh
#define X_SWITCH_TYPE_MIN       SWITCH_TYPE
#define X_SWITCH_TYPE_MAX       SWITCH_TYPE

#define Y_AXIS_MODE 			AXIS_STANDARD
#define Y_VELOCITY_MAX 			18000.0
#define Y_FEEDRATE_MAX 			Y_VELOCITY_MAX
#define Y_TRAVEL_MIN 			0
#define Y_TRAVEL_MAX 			190
#define Y_JERK_MAX 				40000.0
#define Y_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define Y_SWITCH_MODE_MIN		SW_MODE_HOMING
#define Y_SWITCH_MODE_MAX		SW_MODE_LIMIT
#define Y_SEARCH_VELOCITY 		3000
#define Y_LATCH_VELOCITY		200
#define Y_LATCH_BACKOFF			10
#define Y_ZERO_BACKOFF			3
#define Y_JERK_HOMING			Y_JERK_MAX
#define Y_SWITCH_TYPE_MIN       SWITCH_TYPE
#define Y_SWITCH_TYPE_MAX       SWITCH_TYPE

#define Z_AXIS_MODE				AXIS_STANDARD
#define Z_VELOCITY_MAX			800
#define Z_FEEDRATE_MAX			Z_VELOCITY_MAX
#define Z_TRAVEL_MIN 			0
#define Z_TRAVEL_MAX			220
#define Z_JERK_MAX				50				// 50,000,000
#define Z_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define Z_SWITCH_MODE_MIN       SW_MODE_HOMING
#define Z_SWITCH_MODE_MAX       SW_MODE_DISABLED
#define Z_SEARCH_VELOCITY		200
#define Z_LATCH_VELOCITY		100
#define Z_LATCH_BACKOFF			5
#define Z_ZERO_BACKOFF			-0.5
#define Z_JERK_HOMING			Z_JERK_MAX
#define Z_SWITCH_TYPE_MIN       SWITCH_TYPE
#define Z_SWITCH_TYPE_MAX       SWITCH_TYPE


/******************************************************
 * To calulate the speeds here, in Wolfram Alpha-speak:
 * c=2*pi*r, r=0.609, d=c/360, s=((S*60)/d), S=40 for s
 * Change r to A_RADIUS, and S to the desired speed,
 * in mm/s or mm/s/s/s.
 *
 * It will return s= as the value you want to enter.
 *
 * If the value is over 1 million, the code will divide
 * it by 1 million, so you have to pre-multiply it by
 * 1000000.0. (The value is in millions, btw.)
 *
 * Note that you need these to be floating point values,
 * so always have a .0 at the end!
 *
 ******************************************************/

#define A_AXIS_MODE 			AXIS_RADIUS
#define A_RADIUS 				0.609
#define A_VELOCITY_MAX          225769.0 // ~40 mm/s, 2,400 mm/min
#define A_FEEDRATE_MAX 			225769.0 // ~20 mm/s, 1,200 mm/min
#define A_TRAVEL_MIN 			0
#define A_TRAVEL_MAX 			10
#define A_JERK_MAX 				1128980.0*1000000.0 // 2,000 million mm/min^3
                                                  // * a million since it's over a million
// c=2*pi*r, r=0.609, d=c/360, s=((2000*60)/d)
#define A_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define A_SWITCH_MODE_MIN		SW_MODE_DISABLED
#define A_SWITCH_MODE_MAX		SW_MODE_DISABLED
#define A_SEARCH_VELOCITY 		2000
#define A_LATCH_VELOCITY 		2000
#define A_LATCH_BACKOFF 		5
#define A_ZERO_BACKOFF 			2
#define A_JERK_HOMING			A_JERK_MAX
#define A_SWITCH_TYPE_MIN       SWITCH_TYPE
#define A_SWITCH_TYPE_MAX       SWITCH_TYPE

#define B_AXIS_MODE				AXIS_DISABLED
#define B_VELOCITY_MAX			3600
#define B_FEEDRATE_MAX			B_VELOCITY_MAX
#define B_TRAVEL_MIN 			0
#define B_TRAVEL_MAX			-1
//#define B_JERK_MAX				20000000
#define B_JERK_MAX				20
#define B_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define B_RADIUS				1
#define B_SWITCH_MODE_MIN		SW_MODE_DISABLED
#define B_SWITCH_MODE_MAX		SW_MODE_DISABLED
#define B_SEARCH_VELOCITY 		600
#define B_LATCH_VELOCITY 		100
#define B_LATCH_BACKOFF			10
#define B_ZERO_BACKOFF			2
#define B_JERK_HOMING			A_JERK_MAX
#define B_SWITCH_TYPE_MIN       SWITCH_TYPE
#define B_SWITCH_TYPE_MAX       SWITCH_TYPE

#define C_AXIS_MODE				AXIS_DISABLED
#define C_VELOCITY_MAX			3600
#define C_FEEDRATE_MAX			C_VELOCITY_MAX
#define C_TRAVEL_MIN 			0
#define C_TRAVEL_MAX			-1
//#define C_JERK_MAX				20000000
#define C_JERK_MAX				20
#define C_JUNCTION_DEVIATION	JUNCTION_DEVIATION
#define C_RADIUS				1
#define C_SWITCH_MODE_MIN		SW_MODE_DISABLED
#define C_SWITCH_MODE_MAX		SW_MODE_DISABLED
#define C_SEARCH_VELOCITY 		600
#define C_LATCH_VELOCITY 		100
#define C_LATCH_BACKOFF			10
#define C_ZERO_BACKOFF			2
#define C_JERK_HOMING			A_JERK_MAX
#define C_SWITCH_TYPE_MIN       SWITCH_TYPE
#define C_SWITCH_TYPE_MAX       SWITCH_TYPE

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


