/*
 * settings_shopbot_sbv300.h - ShopBot sbv3000
 * This file is part of the g2core project
 *
 * Copyright (c) 2015 - 2018 Alden S. Hart, Jr.
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
/**** ShopBot profile ******************************************/
/***********************************************************************/
/*
 * NOTES:
 *  - This profile supports the ShopBot sbv30x board
 */

// ***> NOTE: The init message must be a single line with no CRs or LFs
#define INIT_MESSAGE "Initializing configs to ShopBot sbv300 profile"

//**** GLOBAL / GENERAL SETTINGS ******************************************************

#define JUNCTION_INTEGRATION_TIME   1.5     // cornering - between 0.10 and 2.00 (higher is faster)
#define CHORDAL_TOLERANCE           0.01    // chordal accuracy for arc drawing (in mm) 

#define HAS_LASER                   0       // We don't have a laser ... yet

#define SOFT_LIMIT_ENABLE           0       // 0=off, 1=on
#define HARD_LIMIT_ENABLE           1       // 0=off, 1=on    ////## should be on by default???
#define SAFETY_INTERLOCK_ENABLE     1       // 0=off, 1=on

////## #define SPINDLE_ENABLE_POLARITY     1       // {spep: 0=active low, 1=active high
#define SPINDLE_ENABLE_POLARITY     0       // {spep: 0=active high, 1=active low
#define SPINDLE_DIR_POLARITY        0       // 0=clockwise is low, 1=clockwise is high
#define SPINDLE_PAUSE_ON_HOLD       true
#define SPINDLE_SPINUP_DELAY        2.0
#define SPINDLE_SPEED_CHANGE_PER_MS 7.0

#define COOLANT_MIST_POLARITY       1       // 0=active low, 1=active high
#define COOLANT_FLOOD_POLARITY      1       // 0=active low, 1=active high
#define COOLANT_PAUSE_ON_HOLD       true

#define MIST_ENABLE_OUTPUT_NUMBER   0
#define FLOOD_ENABLE_OUTPUT_NUMBER  0

#define SPINDLE_ENABLE_OUTPUT_NUMBER    1   ////## this is also defined in board_gpio ... probably a mistake
#define SPINDLE_DIRECTION_OUTPUT_NUMBER 0
#define SPINDLE_PWM_NUMBER              0

#define FEEDHOLD_Z_LIFT             12.7

#define MANUAL_FEEDRATE_OVERRIDE_ENABLE false
#define MANUAL_FEEDRATE_OVERRIDE_PARAMETER 1.00

// Communications and reporting settings

#define USB_SERIAL_PORTS_EXPOSED    1                   // Valid options are 1 or 2, only!

#define COMM_MODE                   JSON_MODE           // one of: TEXT_MODE, JSON_MODE
#define XIO_ENABLE_FLOW_CONTROL     FLOW_CONTROL_RTS    // FLOW_CONTROL_OFF, FLOW_CONTROL_RTS

#define TEXT_VERBOSITY              TV_VERBOSE          // one of: TV_SILENT, TV_VERBOSE
#define JSON_VERBOSITY              JV_CONFIGS          // one of: JV_SILENT, JV_FOOTER, JV_CONFIGS, JV_MESSAGES, JV_LINENUM, JV_VERBOSE
#define QUEUE_REPORT_VERBOSITY      QR_OFF              // one of: QR_OFF, QR_SINGLE, QR_TRIPLE  ////##-to off

#define STATUS_REPORT_VERBOSITY     SR_FILTERED         // one of: SR_OFF, SR_FILTERED, SR_VERBOSE
#define STATUS_REPORT_MIN_MS        100                 // milliseconds - enforces a viable minimum
#define STATUS_REPORT_INTERVAL_MS   250                 // milliseconds - set $SV=0 to disable

////##added c, spc, inputs, and outputs (tests from Matt are in FabMo config)
#define STATUS_REPORT_DEFAULTS "posx", "posy", "posz", "posa", "posb", "posc", \
                               "vel", "stat", "hold", "line", "coor", "unit", "spc",\
                               "in1", "in2", "in3", "in4", "in5", "in6", "in7", "in8", "in9","in10","in11","in12",\
                               "out1", "out2", "out3", "out4", "out5", "out6", "out7", "out8", "out9", "out10", "out11", "out12" 	

// Gcode startup defaults
#define GCODE_DEFAULT_UNITS         MILLIMETERS              // MILLIMETERS or INCHES   ////** note not consistent with distance values ??? all reset???
#define GCODE_DEFAULT_PLANE         CANON_PLANE_XY      // CANON_PLANE_XY, CANON_PLANE_XZ, or CANON_PLANE_YZ
// #define GCODE_DEFAULT_COORD_SYSTEM  G55                 // G54, G55, G56, G57, G58 or G59
#define GCODE_DEFAULT_COORD_SYSTEM  G54                 // G54, G55, G56, G57, G58 or G59
#define GCODE_DEFAULT_PATH_CONTROL  PATH_CONTINUOUS
#define GCODE_DEFAULT_DISTANCE_MODE ABSOLUTE_DISTANCE_MODE

// *** motor settings ORIGINAL (modified at some point?)************************************************************************************

// #define MOTOR_POWER_MODE            MOTOR_POWERED_IN_CYCLE  // default motor power mode (see cmMotorPowerMode in stepper.h)
// #define MOTOR_POWER_TIMEOUT         2.00                // motor power timeout in seconds
// #define MOTOR_POWER_LEVEL           0.375               // default motor power level 0.00 - 1.00

// #define M1_MOTOR_MAP                AXIS_X_EXTERNAL     // 1ma
// #define M1_STEP_ANGLE               1.8                 // 1sa
// #define M1_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)        // 1tr
// #define M1_MICROSTEPS               10                  // 1mi        1,2,4,8
// #define M1_POLARITY                 0                   // 1po        0=normal, 1=reversed
// #define M1_POWER_MODE               MOTOR_POWER_MODE    // 1pm        TRUE=low power idle enabled
// #define M1_POWER_LEVEL              MOTOR_POWER_LEVEL   // 1pl        Irrelevant to Shopbot sbv300

// #define M2_MOTOR_MAP                AXIS_Y_EXTERNAL
// #define M2_STEP_ANGLE               1.8
// #define M2_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)
// #define M2_MICROSTEPS               10
// #define M2_POLARITY                 0
// #define M2_POWER_MODE               MOTOR_POWER_MODE
// #define M2_POWER_LEVEL              MOTOR_POWER_LEVEL

// #define M3_MOTOR_MAP                AXIS_Z_EXTERNAL
// #define M3_STEP_ANGLE               1.8
// #define M3_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)
// #define M3_MICROSTEPS               10
// #define M3_POLARITY                 0
// #define M3_POWER_MODE               MOTOR_POWER_MODE
// #define M3_POWER_LEVEL              MOTOR_POWER_LEVEL

// #define M4_MOTOR_MAP                AXIS_A_EXTERNAL
// #define M4_STEP_ANGLE               1.8
// #define M4_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)
// #define M4_MICROSTEPS               10
// #define M4_POLARITY                 0
// #define M4_POWER_MODE               MOTOR_POWER_MODE
// #define M4_POWER_LEVEL              MOTOR_POWER_LEVEL

// #define M5_MOTOR_MAP                AXIS_B_EXTERNAL
// #define M5_STEP_ANGLE               1.8
// #define M5_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)
// #define M5_MICROSTEPS               10
// #define M5_POLARITY                 0
// #define M5_POWER_MODE               MOTOR_POWER_MODE
// #define M5_POWER_LEVEL              MOTOR_POWER_LEVEL

// #define M6_MOTOR_MAP                AXIS_C_EXTERNAL
// #define M6_STEP_ANGLE               1.8
// #define M6_TRAVEL_PER_REV           0.5 ////## to inches(0.5 * 25.4)
// #define M6_MICROSTEPS               10
// #define M6_POLARITY                 0
// #define M6_POWER_MODE               MOTOR_POWER_MODE
// #define M6_POWER_LEVEL              MOTOR_POWER_LEVEL


// *** SIMPLIFIED METRIC (su=157.48031) ************[su=100 provides a simplified version with travel @ 20]*************

#define MOTOR_POWER_MODE            MOTOR_POWERED_IN_CYCLE  // default motor power mode (see cmMotorPowerMode in stepper.h)
#define MOTOR_POWER_TIMEOUT         2.00                // motor power timeout in seconds
#define MOTOR_POWER_LEVEL           0.375               // default motor power level 0.00 - 1.00

#define M1_MOTOR_MAP                AXIS_X_EXTERNAL     // 1ma
#define M1_STEP_ANGLE               1.8                 // 1sa
#define M1_TRAVEL_PER_REV           12.7
#define M1_MICROSTEPS               10                  // 1mi        1,2,4,8
#define M1_POLARITY                 0                   // 1po        0=normal, 1=reversed
#define M1_POWER_MODE               MOTOR_POWER_MODE    // 1pm        TRUE=low power idle enabled
#define M1_POWER_LEVEL              MOTOR_POWER_LEVEL   // 1pl        Irrelevant to Shopbot sbv300

#define M2_MOTOR_MAP                AXIS_Y_EXTERNAL
#define M2_STEP_ANGLE               1.8
#define M2_TRAVEL_PER_REV           12.7
#define M2_MICROSTEPS               10
#define M2_POLARITY                 0
#define M2_POWER_MODE               MOTOR_POWER_MODE
#define M2_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M3_MOTOR_MAP                AXIS_Z_EXTERNAL
#define M3_STEP_ANGLE               1.8
#define M3_TRAVEL_PER_REV           12.7
#define M3_MICROSTEPS               10
#define M3_POLARITY                 0
#define M3_POWER_MODE               MOTOR_POWER_MODE
#define M3_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M4_MOTOR_MAP                AXIS_A_EXTERNAL
#define M4_STEP_ANGLE               1.8
#define M4_TRAVEL_PER_REV           360
#define M4_MICROSTEPS               10
#define M4_POLARITY                 0
#define M4_POWER_MODE               MOTOR_POWER_MODE
#define M4_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M5_MOTOR_MAP                AXIS_B_EXTERNAL
#define M5_STEP_ANGLE               1.8
#define M5_TRAVEL_PER_REV           360
#define M5_MICROSTEPS               10
#define M5_POLARITY                 0
#define M5_POWER_MODE               MOTOR_POWER_MODE
#define M5_POWER_LEVEL              MOTOR_POWER_LEVEL

#define M6_MOTOR_MAP                AXIS_C_EXTERNAL
#define M6_STEP_ANGLE               1.8
#define M6_TRAVEL_PER_REV           360
#define M6_MICROSTEPS               10
#define M6_POLARITY                 0
#define M6_POWER_MODE               MOTOR_POWER_MODE
#define M6_POWER_LEVEL              MOTOR_POWER_LEVEL


// // *** axis settings ORIGINAL (modified at some point?)*********************************************************************************

// #define X_AXIS_MODE                 AXIS_STANDARD   // xam  see canonical_machine.h cmAxisMode for valid values
// #define X_VELOCITY_MAX              360 ////## to inches (360 * 25.4)    // xvm  G0 max velocity in mm/min
// #define X_FEEDRATE_MAX              X_VELOCITY_MAX  // xfr  G1 max feed rate in mm/min
// #define X_TRAVEL_MIN                0               // xtn  minimum travel for soft limits
// #define X_TRAVEL_MAX                24 ////## to inches (25 * 25.4)     // xtm  travel between switches or crashes
// #define X_JERK_MAX                  50 ////## to inches (2 * 25.4)      // xjm  jerk is multiplied by 1,000,000 internally
// #define X_JERK_HIGH_SPEED           10000           // xjh
// #define X_HOMING_INPUT              3               // xhi  input used for homing or 0 to disable
// #define X_HOMING_DIRECTION          0               // xhd  0=search moves negative, 1= search moves positive
// #define X_SEARCH_VELOCITY           60 ////## to inches (60 * 25.4)     // xsv  minus means move to minimum switch
// #define X_LATCH_VELOCITY            30 ////## to inches (30 * 25.4)     // xlv  mm/min
// #define X_LATCH_BACKOFF             0.25 ////## to inches (0.125 * 25.4)  // xlb  mm
// #define X_ZERO_BACKOFF              0.25 ////## (0.375 * 25.4)  // xzb  mm

// #define Y_AXIS_MODE                 AXIS_STANDARD
// #define Y_VELOCITY_MAX              360 ////##(360 * 25.4)
// #define Y_FEEDRATE_MAX              Y_VELOCITY_MAX
// #define Y_TRAVEL_MIN                0
// #define Y_TRAVEL_MAX                18 ////##(19 * 25.4)
// #define Y_JERK_MAX                  50 ////##(2 * 25.4)
// #define Y_JERK_HIGH_SPEED           10000
// #define Y_HOMING_INPUT              3
// #define Y_HOMING_DIRECTION          0
// #define Y_SEARCH_VELOCITY           60 ////##(60 * 25.4)
// #define Y_LATCH_VELOCITY            30 ////##(30 * 25.4)
// #define Y_LATCH_BACKOFF             0.25 ////##(0.125 * 25.4)
// #define Y_ZERO_BACKOFF              0.25 ////##(0.375 * 25.4)

// #define Z_AXIS_MODE                 AXIS_STANDARD
// #define Z_VELOCITY_MAX              360 ////##(360 * 25.4)
// #define Z_FEEDRATE_MAX              Z_VELOCITY_MAX
// #define Z_TRAVEL_MAX                6.5 ////##(6.5 * 25.4)
// #define Z_TRAVEL_MIN                0
// #define Z_JERK_MAX                  50 ////##(2 * 25.4)
// #define Z_JERK_HIGH_SPEED           1000
// #define Z_HOMING_INPUT              5
// #define Z_HOMING_DIRECTION          1
// #define Z_SEARCH_VELOCITY           60 ////##(60 * 25.4)
// #define Z_LATCH_VELOCITY            30 ////##(30 * 25.4)
// #define Z_LATCH_BACKOFF             0.25 ////##(0.125 * 25.4)
// #define Z_ZERO_BACKOFF              0.25 ////##(0.375 * 25.4)

// #define A_AXIS_MODE                 AXIS_STANDARD
// #define A_VELOCITY_MAX              3600 ////##(360 * 25.4)
// #define A_FEEDRATE_MAX              4800
// #define A_TRAVEL_MIN                -1  // degrees
// #define A_TRAVEL_MAX                -1  // same value means infinite, no limit
// #define A_JERK_MAX                  200 ////##(2 * 25.4)
// #define A_JERK_HIGH_SPEED           A_JERK_MAX
// #define A_RADIUS                    1.0
// #define A_HOMING_INPUT              0
// #define A_HOMING_DIRECTION          0
// #define A_SEARCH_VELOCITY           1000 ////##(60 * 25.4)
// #define A_LATCH_VELOCITY            500 ////##(30 * 25.4)
// #define A_LATCH_BACKOFF             0.25 ////##(0.125 * 25.4)
// #define A_ZERO_BACKOFF              0.25 ////##(0.375 * 25.4)

// #define B_AXIS_MODE                 AXIS_STANDARD
// #define B_VELOCITY_MAX              3600 ////##(360 * 25.4)
// #define B_FEEDRATE_MAX              4800
// #define B_TRAVEL_MAX                -1
// #define B_TRAVEL_MIN                -1
// #define B_JERK_MAX                  200 ////##(2 * 25.4)
// #define B_JERK_HIGH_SPEED           B_JERK_MAX
// #define B_RADIUS                    1
// #define B_HOMING_INPUT              0
// #define B_HOMING_DIRECTION          0
// #define B_SEARCH_VELOCITY           1000 ////##(60 * 25.4)
// #define B_LATCH_VELOCITY            500 ////##(30 * 25.4)
// #define B_LATCH_BACKOFF             0.25 ////##(0.125 * 25.4)
// #define B_ZERO_BACKOFF              0.25 ////##(0.375 * 25.4)

// #define C_AXIS_MODE                 AXIS_STANDARD
// #define C_VELOCITY_MAX              3600 ////##(360 * 25.4)
// #define C_FEEDRATE_MAX              4800
// #define C_TRAVEL_MAX                -1
// #define C_TRAVEL_MIN                -1
// #define C_JERK_MAX                  200 ////##(2 * 25.4)
// #define C_JERK_HIGH_SPEED           C_JERK_MAX
// #define C_RADIUS                    1
// #define C_HOMING_INPUT              0
// #define C_HOMING_DIRECTION          0
// #define C_SEARCH_VELOCITY           1000 ////##(60 * 25.4)
// #define C_LATCH_VELOCITY            500 ////##(30 * 25.4)
// #define C_LATCH_BACKOFF             0.25 ////##(0.125 * 25.4)
// #define C_ZERO_BACKOFF              0.25 ////##(0.375 * 25.4)


// *** axis settings  METRIC ************************************************************************************

#define X_AXIS_MODE                 AXIS_STANDARD   // xam  see canonical_machine.h cmAxisMode for valid values
#define X_VELOCITY_MAX              9000           // xvm  G0 max velocity in mm/min
#define X_FEEDRATE_MAX              X_VELOCITY_MAX  // xfr  G1 max feed rate in mm/min
#define X_TRAVEL_MIN                0               // xtn  minimum travel for soft limits
#define X_TRAVEL_MAX                635             // xtm  travel between switches or crashes
#define X_JERK_MAX                  1000            // xjm  jerk is multiplied by 1,000,000 internally
#define X_JERK_HIGH_SPEED           10000           // xjh
#define X_HOMING_INPUT              0               // xhi  input used for homing or 0 to disable
#define X_HOMING_DIRECTION          0               // xhd  0=search moves negative, 1= search moves positive
#define X_SEARCH_VELOCITY           1500            // xsv  minus means move to minimum switch
#define X_LATCH_VELOCITY            500             // xlv  mm/min
#define X_LATCH_BACKOFF             5               // xlb  mm
#define X_ZERO_BACKOFF              5               // xzb  mm

#define Y_AXIS_MODE                 AXIS_STANDARD
#define Y_VELOCITY_MAX              9000
#define Y_FEEDRATE_MAX              Y_VELOCITY_MAX
#define Y_TRAVEL_MIN                0
#define Y_TRAVEL_MAX                460
#define Y_JERK_MAX                  1000
#define Y_JERK_HIGH_SPEED           10000
#define Y_HOMING_INPUT              0
#define Y_HOMING_DIRECTION          0
#define Y_SEARCH_VELOCITY           1500
#define Y_LATCH_VELOCITY            500
#define Y_LATCH_BACKOFF             5
#define Y_ZERO_BACKOFF              5

#define Z_AXIS_MODE                 AXIS_STANDARD
#define Z_VELOCITY_MAX              9000
#define Z_FEEDRATE_MAX              Z_VELOCITY_MAX
#define Z_TRAVEL_MIN                -25
#define Z_TRAVEL_MAX                150
#define Z_JERK_MAX                  1000
#define Z_JERK_HIGH_SPEED           10000
#define Z_HOMING_INPUT              0
#define Z_HOMING_DIRECTION          0
#define Z_SEARCH_VELOCITY           1500
#define Z_LATCH_VELOCITY            500
#define Z_LATCH_BACKOFF             5
#define Z_ZERO_BACKOFF              5

#define A_AXIS_MODE                 AXIS_STANDARD
#define A_VELOCITY_MAX              21600
#define A_FEEDRATE_MAX              21600
#define A_TRAVEL_MIN                -1  // degrees
#define A_TRAVEL_MAX                -1  // same value means infinite, no limit
#define A_JERK_MAX                  9000
#define A_JERK_HIGH_SPEED           90000
#define A_RADIUS                    1.0
#define A_HOMING_INPUT              0
#define A_HOMING_DIRECTION          0
#define A_SEARCH_VELOCITY           10000
#define A_LATCH_VELOCITY            5000
#define A_LATCH_BACKOFF             50
#define A_ZERO_BACKOFF              50

#define B_AXIS_MODE                 AXIS_STANDARD
#define B_VELOCITY_MAX              21600
#define B_FEEDRATE_MAX              21600
#define B_TRAVEL_MAX                -1
#define B_TRAVEL_MIN                -1
#define B_JERK_MAX                  9000
#define B_JERK_HIGH_SPEED           90000
#define B_RADIUS                    1.0
#define B_HOMING_INPUT              0
#define B_HOMING_DIRECTION          0
#define B_SEARCH_VELOCITY           10000
#define B_LATCH_VELOCITY            5000
#define B_LATCH_BACKOFF             50
#define B_ZERO_BACKOFF              50

#define C_AXIS_MODE                 AXIS_STANDARD
#define C_VELOCITY_MAX              21600
#define C_FEEDRATE_MAX              21600
#define C_TRAVEL_MAX                -1
#define C_TRAVEL_MIN                -1
#define C_JERK_MAX                  9000
#define C_JERK_HIGH_SPEED           90000
#define C_RADIUS                    1.0
#define C_HOMING_INPUT              0
#define C_HOMING_DIRECTION          0
#define C_SEARCH_VELOCITY           10000
#define C_LATCH_VELOCITY            5000
#define C_LATCH_BACKOFF             50
#define C_ZERO_BACKOFF              50




//*** Input / output settings ***

// Inputs

#define DI1_ENABLED IO_ENABLED
#define DI1_POLARITY IO_ACTIVE_LOW
#define DI1_ACTION INPUT_ACTION_NONE

#define DI2_ENABLED IO_ENABLED
#define DI2_POLARITY IO_ACTIVE_LOW
#define DI2_ACTION INPUT_ACTION_NONE

#define DI3_ENABLED IO_ENABLED
#define DI3_POLARITY IO_ACTIVE_LOW
#define DI3_ACTION INPUT_ACTION_NONE

#define DI4_ENABLED IO_ENABLED
#define DI4_POLARITY IO_ACTIVE_LOW
#define DI4_ACTION INPUT_ACTION_STOP   //#### All ShopBots INP-4 ==> STOP ... maybe FAST_STOP

#define DI5_ENABLED IO_ENABLED
#define DI5_POLARITY IO_ACTIVE_LOW
#define DI5_ACTION INPUT_ACTION_NONE

#define DI6_ENABLED IO_ENABLED
#define DI6_POLARITY IO_ACTIVE_LOW
#define DI6_ACTION INPUT_ACTION_NONE

#define DI7_ENABLED IO_ENABLED
#define DI7_POLARITY IO_ACTIVE_LOW
#define DI7_ACTION INPUT_ACTION_NONE

#define DI8_ENABLED IO_ENABLED
#define DI8_POLARITY IO_ACTIVE_LOW
#define DI8_ACTION INPUT_ACTION_NONE

#define DI9_ENABLED IO_ENABLED
#define DI9_POLARITY IO_ACTIVE_LOW
#define DI9_ACTION INPUT_ACTION_NONE

#define DI10_ENABLED IO_ENABLED
#define DI10_POLARITY IO_ACTIVE_LOW
#define DI10_ACTION INPUT_ACTION_NONE

#define DI11_ENABLED IO_ENABLED
#define DI11_POLARITY IO_ACTIVE_LOW
#define DI11_ACTION INPUT_ACTION_NONE

#define DI12_ENABLED IO_ENABLED
#define DI12_POLARITY IO_ACTIVE_LOW
#define DI12_ACTION INPUT_ACTION_NONE

//Outputs

#define DO1_ENABLED IO_ENABLED
#define DO1_POLARITY IO_ACTIVE_HIGH

#define DO2_ENABLED IO_ENABLED
#define DO2_POLARITY IO_ACTIVE_HIGH

#define DO3_ENABLED IO_ENABLED
#define DO3_POLARITY IO_ACTIVE_HIGH

#define DO4_ENABLED IO_ENABLED
#define DO4_POLARITY IO_ACTIVE_HIGH

#define DO5_ENABLED IO_ENABLED
#define DO5_POLARITY IO_ACTIVE_LOW   ////** Changed for Colored-LED System; maybe do per tool?

#define DO6_ENABLED IO_ENABLED
#define DO6_POLARITY IO_ACTIVE_HIGH

#define DO7_ENABLED IO_ENABLED
#define DO7_POLARITY IO_ACTIVE_HIGH

#define DO8_ENABLED IO_ENABLED
#define DO8_POLARITY IO_ACTIVE_HIGH

#define DO9_ENABLED IO_ENABLED
#define DO9_POLARITY IO_ACTIVE_HIGH

#define DO10_ENABLED IO_ENABLED
#define DO10_POLARITY IO_ACTIVE_HIGH

#define DO11_ENABLED IO_ENABLED
#define DO11_POLARITY IO_ACTIVE_HIGH

#define DO12_ENABLED IO_ENABLED
#define DO12_POLARITY IO_ACTIVE_HIGH


/*** Handle optional modules that may not be in every machine ***/

#define P1_PWM_FREQUENCY        100  // in Hz
#define P1_CW_SPEED_LO          7900   // in RPM (arbitrary units)
#define P1_CW_SPEED_HI          12800
#define P1_CW_PHASE_LO          0.13  // phase [0..1]
#define P1_CW_PHASE_HI          0.17
#define P1_CCW_SPEED_LO         0
#define P1_CCW_SPEED_HI         0
#define P1_CCW_PHASE_LO         0.1
#define P1_CCW_PHASE_HI         0.1
#define P1_PWM_PHASE_OFF        0.1

