/*
 * settings_shopbot_test.h - Shopbot Test
 * This file is part of the g2core project
 *
 * Copyright (c) 2015 - 2017 Alden S. Hart, Jr.
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
/**** Shopbot Test profile ******************************************/
/***********************************************************************/
/*
 * NOTES:
 *  - uSteps are set to 8 so they can drive the TI drivers. Shopbot uses mi=10
 *  - Switches are set NO so it can be tested w no switches attached. SB uses NC switches
 */

// ***> NOTE: The init message must be a single line with no CRs or LFs
#define INIT_MESSAGE "Initializing configs to Shopbot test profile"

//**** GLOBAL / GENERAL SETTINGS ******************************************************

#define JUNCTION_INTEGRATION_TIME   0.8     // cornering - between 0.10 and 2.00 (higher is faster)
#define CHORDAL_TOLERANCE           0.0001  // chordal accuracy for arc drawing (in mm)

#define SOFT_LIMIT_ENABLE           0       // 0=off, 1=on
#define HARD_LIMIT_ENABLE           0       // 0=off, 1=on
#define SAFETY_INTERLOCK_ENABLE     0       // 0=off, 1=on

#define SPINDLE_ENABLE_POLARITY     1       // 0=active low, 1=active high
#define SPINDLE_DIR_POLARITY        0       // 0=clockwise is low, 1=clockwise is high
#define SPINDLE_PAUSE_ON_HOLD       true
#define SPINDLE_DWELL_TIME          1.0

#define COOLANT_MIST_POLARITY       1       // 0=active low, 1=active high
#define COOLANT_FLOOD_POLARITY      1       // 0=active low, 1=active high
#define COOLANT_PAUSE_ON_HOLD       true

#define FEEDHOLD_Z_LIFT (0.5 * 25.4)

#define MANUAL_FEEDRATE_OVERRIDE_ENABLE false
#define MANUAL_FEEDRATE_OVERRIDE_PARAMETER 1.00

// Communications and reporting settings

#define USB_SERIAL_PORTS_EXPOSED    1                   // Valid options are 1 or 2, only!

#define COMM_MODE                   JSON_MODE           // one of: TEXT_MODE, JSON_MODE
#define XIO_ENABLE_FLOW_CONTROL     FLOW_CONTROL_RTS    // FLOW_CONTROL_OFF, FLOW_CONTROL_RTS

#define TEXT_VERBOSITY              TV_VERBOSE          // one of: TV_SILENT, TV_VERBOSE
#define JSON_VERBOSITY              JV_VERBOSE          // one of: JV_SILENT, JV_FOOTER, JV_CONFIGS, JV_MESSAGES, JV_LINENUM, JV_VERBOSE
#define QUEUE_REPORT_VERBOSITY      QR_OFF              // one of: QR_OFF, QR_SINGLE, QR_TRIPLE

#define STATUS_REPORT_VERBOSITY     SR_FILTERED         // one of: SR_OFF, SR_FILTERED, SR_VERBOSE
#define STATUS_REPORT_MIN_MS        200                 // milliseconds - enforces a viable minimum
#define STATUS_REPORT_INTERVAL_MS   250                 // milliseconds

#define STATUS_REPORT_DEFAULTS "posx", "posy", "posz", "posa", "posb", \
                               "vel",  "stat", "hold", "line", "coor", "unit", \
                               "in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8"
//                               "home","homx","homy","homz"

// Gcode startup defaults
#define GCODE_DEFAULT_UNITS         INCHES              // MILLIMETERS or INCHES
#define GCODE_DEFAULT_PLANE         CANON_PLANE_XY      // CANON_PLANE_XY, CANON_PLANE_XZ, or CANON_PLANE_YZ
#define GCODE_DEFAULT_COORD_SYSTEM  G55                 // G54, G55, G56, G57, G58 or G59
#define GCODE_DEFAULT_PATH_CONTROL  PATH_CONTINUOUS
#define GCODE_DEFAULT_DISTANCE_MODE ABSOLUTE_DISTANCE_MODE

// *** motor settings ************************************************************************************

#define MOTOR_POWER_MODE            MOTOR_POWERED_IN_CYCLE  // default motor power mode (see cmMotorPowerMode in stepper.h)
#define MOTOR_POWER_TIMEOUT         2.00                // motor power timeout in seconds
#define MOTOR_POWER_LEVEL           0.375               // default motor power level 0.00 - 1.00

#define M1_MOTOR_MAP                AXIS_X              // 1ma
#define M1_STEP_ANGLE               1.8                 // 1sa
#define M1_TRAVEL_PER_REV           (0.5 * 25.4)        // 1tr
#define M1_MICROSTEPS               8                   // 1mi        1,2,4,8
#define M1_POLARITY                 1                   // 1po        0=normal, 1=reversed
#define M1_POWER_MODE               MOTOR_POWER_MODE    // 1pm        TRUE=low power idle enabled
#define M1_POWER_LEVEL              MOTOR_POWER_LEVEL   // 1pl        Irrelevant to Shopbot sbv300

#define M2_MOTOR_MAP                AXIS_Y
#define M2_STEP_ANGLE               1.8
#define M2_TRAVEL_PER_REV           (0.5 * 25.4)
#define M2_MICROSTEPS               8
#define M2_POLARITY                 1
#define M2_POWER_MODE               MOTOR_POWER_MODE
#define M2_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M3_MOTOR_MAP                AXIS_Z
#define M3_STEP_ANGLE               1.8
#define M3_TRAVEL_PER_REV           (0.5 * 25.4)
#define M3_MICROSTEPS               8
#define M3_POLARITY                 0
#define M3_POWER_MODE               MOTOR_POWER_MODE
#define M3_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M4_MOTOR_MAP                AXIS_A
#define M4_STEP_ANGLE               1.8
#define M4_TRAVEL_PER_REV           48      // was (0.5 * 25.4)  Set to agree with sbv300 su = 33.333333
#define M4_MICROSTEPS               8
#define M4_POLARITY                 0
#define M4_POWER_MODE               MOTOR_POWER_MODE
#define M4_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M5_MOTOR_MAP                AXIS_B
#define M5_STEP_ANGLE               1.8
#define M5_TRAVEL_PER_REV           48  // (0.5 * 25.4)
#define M5_MICROSTEPS               8
#define M5_POLARITY                 0
#define M5_POWER_MODE               MOTOR_POWER_MODE
#define M5_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M6_MOTOR_MAP                AXIS_C
#define M6_STEP_ANGLE               1.8
#define M6_TRAVEL_PER_REV           48  // (0.5 * 25.4)
#define M6_MICROSTEPS               8
#define M6_POLARITY                 0
#define M6_POWER_MODE               MOTOR_POWER_MODE
#define M6_POWER_LEVEL              MOTOR_POWER_LEVEL

// *** axis settings *********************************************************************************

// Axis values are set to make the Othermachine behave like a smaller Handibot
// Units are not to scale: i.e. G0 X1 travels approx 0.34" instead of 1"

#define LINEAR_JERK_MAX             50              // value set by sbv300 initialization sequence
//#define LINEAR_JERK_MAX             25.4            // test value
#define ROTARY_JERK_MAX             6000            // sbv300 initialization value

#define X_AXIS_MODE                 AXIS_STANDARD   // xam  see canonical_machine.h cmAxisMode for valid values
#define X_VELOCITY_MAX              (100 * 25.4)    // xvm  G0 max velocity in mm/min (was 360)
#define X_FEEDRATE_MAX              X_VELOCITY_MAX  // xfr  G1 max feed rate in mm/min
#define X_TRAVEL_MIN                0               // xtn  minimum travel for soft limits
#define X_TRAVEL_MAX                (14 * 25.4)     // xtm  travel between switches or crashes (was 25)
#define X_JERK_MAX                  LINEAR_JERK_MAX // xjm
#define X_JERK_HIGH_SPEED           10000           // xjh
#define X_HOMING_INPUT              1               // xhi  input used for homing or 0 to disable
#define X_HOMING_DIRECTION          0               // xhd  0=search moves negative, 1= search moves positive
#define X_SEARCH_VELOCITY           (60 * 25.4)     // xsv  minus means move to minimum switch
#define X_LATCH_VELOCITY            (30 * 25.4)     // xlv  mm/min
#define X_LATCH_BACKOFF             (0.125 * 25.4)  // xlb  mm
#define X_ZERO_BACKOFF              (0.250 * 25.4)  // xzb  mm was 0.375

#define Y_AXIS_MODE                 AXIS_STANDARD
#define Y_VELOCITY_MAX              (100 * 25.4)    // was 360
#define Y_FEEDRATE_MAX              Y_VELOCITY_MAX
#define Y_TRAVEL_MIN                0
#define Y_TRAVEL_MAX                (11.5 * 25.4)   // was 19
#define Y_JERK_MAX                  LINEAR_JERK_MAX
#define Y_JERK_HIGH_SPEED           10000
#define Y_HOMING_INPUT              3
#define Y_HOMING_DIRECTION          0
#define Y_SEARCH_VELOCITY           (60 * 25.4)
#define Y_LATCH_VELOCITY            (30 * 25.4)
#define Y_LATCH_BACKOFF             (0.125 * 25.4)
#define Y_ZERO_BACKOFF              (0.250 * 25.4)  // was 0.375

#define Z_AXIS_MODE                 AXIS_STANDARD
#define Z_VELOCITY_MAX              (100 * 25.4)    // was 360
#define Z_FEEDRATE_MAX              Z_VELOCITY_MAX
#define Z_TRAVEL_MAX                (6 * 25.4)      // was 6.5
#define Z_TRAVEL_MIN                0
#define Z_JERK_MAX                  LINEAR_JERK_MAX
#define Z_JERK_HIGH_SPEED           1000
#define Z_HOMING_INPUT              6
#define Z_HOMING_DIRECTION          1
#define Z_SEARCH_VELOCITY           (60 * 25.4)
#define Z_LATCH_VELOCITY            (30 * 25.4)
#define Z_LATCH_BACKOFF             (0.125 * 25.4)
#define Z_ZERO_BACKOFF              (0.250 * 25.4)  // was 0.375

#define A_AXIS_MODE                 AXIS_STANDARD
#define A_VELOCITY_MAX              (360 * 25.4)    // is 15000 in sbv300
#define A_FEEDRATE_MAX              48000
#define A_TRAVEL_MIN                -1  // degrees
#define A_TRAVEL_MAX                -1  // same value means infinite, no limit
#define A_JERK_MAX                  ROTARY_JERK_MAX
#define A_JERK_HIGH_SPEED           A_JERK_MAX
#define A_RADIUS                    1.0
#define A_HOMING_INPUT              0
#define A_HOMING_DIRECTION          0
#define A_SEARCH_VELOCITY           (60 * 25.4)
#define A_LATCH_VELOCITY            (30 * 25.4)
#define A_LATCH_BACKOFF             (0.125 * 25.4)
#define A_ZERO_BACKOFF              (0.375 * 25.4)

#define B_AXIS_MODE                 AXIS_DISABLED
#define B_VELOCITY_MAX              (360 * 25.4)
#define B_FEEDRATE_MAX              B_VELOCITY_MAX
#define B_TRAVEL_MAX                -1
#define B_TRAVEL_MIN                -1
#define B_JERK_MAX                  ROTARY_JERK_MAX
#define B_JERK_HIGH_SPEED           B_JERK_MAX
#define B_RADIUS                    1
#define B_HOMING_INPUT              0
#define B_HOMING_DIRECTION          0
#define B_SEARCH_VELOCITY           (60 * 25.4)
#define B_LATCH_VELOCITY            (30 * 25.4)
#define B_LATCH_BACKOFF             (0.125 * 25.4)
#define B_ZERO_BACKOFF              (0.375 * 25.4)

#define C_AXIS_MODE                 AXIS_DISABLED
#define C_VELOCITY_MAX              (360 * 25.4)
#define C_FEEDRATE_MAX              C_VELOCITY_MAX
#define C_TRAVEL_MAX                -1
#define C_TRAVEL_MIN                -1
#define C_JERK_MAX                  ROTARY_JERK_MAX
#define C_JERK_HIGH_SPEED           C_JERK_MAX
#define C_RADIUS                    1
#define C_HOMING_INPUT              0
#define C_HOMING_DIRECTION          0
#define C_SEARCH_VELOCITY           (60 * 25.4)
#define C_LATCH_VELOCITY            (30 * 25.4)
#define C_LATCH_BACKOFF             (0.125 * 25.4)
#define C_ZERO_BACKOFF              (0.375 * 25.4)

//*** Input / output settings ***
/*
    See gpio.h GPIO defines for options

    Homing and probing settings are independent of ACTION and FUNCTION settings
    but rely on proper switch MODE setting (i.e. NC or NO)

    INPUT_MODE_DISABLED
    INPUT_ACTIVE_LOW    aka NORMALLY_OPEN
    INPUT_ACTIVE_HIGH   aka NORMALLY_CLOSED

    INPUT_ACTION_NONE
    INPUT_ACTION_STOP
    INPUT_ACTION_FAST_STOP
    INPUT_ACTION_HALT
    INPUT_ACTION_RESET

    INPUT_FUNCTION_NONE
    INPUT_FUNCTION_LIMIT
    INPUT_FUNCTION_INTERLOCK
    INPUT_FUNCTION_SHUTDOWN
    INPUT_FUNCTION_PANIC
*/

// Xmin on v9 board
#define DI1_MODE NORMALLY_CLOSED  // switch is NC type. switch is hit for limit
#define DI1_ACTION INPUT_ACTION_FAST_STOP
#define DI1_FUNCTION INPUT_FUNCTION_LIMIT  // configured as limit switch. Also used for X homing

// Xmax
#define DI2_MODE IO_MODE_DISABLED  // testing disable
//#define DI2_MODE                    NORMALLY_CLOSED       // configured as limit switch
#define DI2_ACTION INPUT_ACTION_FAST_STOP
#define DI2_FUNCTION INPUT_FUNCTION_LIMIT

// Ymin
#define DI3_MODE NORMALLY_CLOSED  // configured as limit switch. ALso used for Y homing
#define DI3_ACTION INPUT_ACTION_FAST_STOP
#define DI3_FUNCTION INPUT_FUNCTION_LIMIT

// Ymax
#define DI4_MODE NORMALLY_CLOSED  // configured as limit switch
#define DI4_ACTION INPUT_ACTION_FAST_STOP
#define DI4_FUNCTION INPUT_FUNCTION_LIMIT

// Zmin
#define DI5_MODE IO_ACTIVE_LOW  // used for Z probe. Active closes circuit (active LO)
#define DI5_ACTION INPUT_ACTION_NONE
#define DI5_FUNCTION INPUT_FUNCTION_NONE

// Zmax
#define DI6_MODE NORMALLY_CLOSED  // configured as shutdown for testing
#define DI6_ACTION INPUT_ACTION_HALT
#define DI6_FUNCTION INPUT_FUNCTION_SHUTDOWN

//#define DI6_MODE                    NORMALLY_CLOSED         // configured as limit switch. ALso used for Z homing
//#define DI6_ACTION                  INPUT_ACTION_FAST_STOP
//#define DI6_FUNCTION                INPUT_FUNCTION_LIMIT

// Amin
#define DI7_MODE IO_ACTIVE_HIGH         // interlock happens when NO switch goes HI (opens)
#define DI7_ACTION INPUT_ACTION_STOP    // wired as interlock switch
#define DI7_FUNCTION INPUT_FUNCTION_INTERLOCK

// Amax
#define DI8_MODE IO_ACTIVE_LOW      // configured as panic switch, NO
#define DI8_ACTION INPUT_ACTION_PANIC
#define DI8_FUNCTION INPUT_FUNCTION_NONE

// Safety line
#define DI9_MODE NORMALLY_CLOSED
#define DI9_ACTION INPUT_ACTION_NONE
#define DI9_FUNCTION INPUT_FUNCTION_NONE

/*** Handle optional modules that may not be in every machine ***/

#define P1_PWM_FREQUENCY 100  // in Hz
#define P1_CW_SPEED_LO 7900   // in RPM (arbitrary units)
#define P1_CW_SPEED_HI 12800
#define P1_CW_PHASE_LO 0.13  // phase [0..1]
#define P1_CW_PHASE_HI 0.17
#define P1_CCW_SPEED_LO 0
#define P1_CCW_SPEED_HI 0
#define P1_CCW_PHASE_LO 0.1
#define P1_CCW_PHASE_HI 0.1
#define P1_PWM_PHASE_OFF 0.1

