/*
 * controller.cpp - tinyg controller and top level parser
 * This file is part of the TinyG project
 *
 * Copyright (c) 2010 - 2014 Alden S. Hart, Jr.
 * Copyright (c) 2013 - 2014 Robert Giseburt
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

#include "tinyg2.h"				// #1
#include "config.h"				// #2
#include "controller.h"
#include "json_parser.h"
#include "text_parser.h"
#include "gcode_parser.h"
#include "canonical_machine.h"
#include "plan_arc.h"
#include "planner.h"
#include "stepper.h"
#include "encoder.h"
#include "hardware.h"
#include "switch.h"
//#include "gpio.h"
#include "report.h"
#include "help.h"
#include "util.h"
#include "xio.h"
#include "settings.h"

#ifdef __ARM
#include "Reset.h"
#endif

/***********************************************************************************
 **** STRUCTURE ALLOCATIONS *********************************************************
 ***********************************************************************************/

controller_t cs;		// controller state structure

/***********************************************************************************
 **** STATICS AND LOCALS ***********************************************************
 ***********************************************************************************/

static void _controller_HSM(void);
static stat_t _shutdown_idler(void);
static stat_t _normal_idler(void);
static stat_t _limit_switch_handler(void);
static stat_t _interlock_estop_handler(void);
static stat_t _system_assertions(void);
static stat_t _sync_to_planner(void);
static stat_t _sync_to_tx_buffer(void);
static stat_t _controller_state(void);
static stat_t _dispatch_command(void);
static stat_t _dispatch_control(void);
static void _dispatch_kernel(void);

// prep for export to other modules:
stat_t hardware_hard_reset_handler(void);
//stat_t hardware_bootloader_handler(void);

/***********************************************************************************
 **** CODE *************************************************************************
 ***********************************************************************************/
/*
 * controller_init() - controller init
 */

void controller_init(uint8_t std_in, uint8_t std_out, uint8_t std_err)
{
	memset(&cs, 0, sizeof(controller_t));			// clear all values, job_id's, pointers and status
	controller_init_assertions();

	cs.fw_build = TINYG_FIRMWARE_BUILD;
	cs.fw_version = TINYG_FIRMWARE_VERSION;
	cs.config_version = TINYG_CONFIG_VERSION;
	cs.hw_platform = TINYG_HARDWARE_PLATFORM;		// NB: HW version is set from EEPROM

	cs.controller_state = CONTROLLER_STARTUP;		// ready to run startup lines

#ifdef __AVR
	xio_set_stdin(std_in);
	xio_set_stdout(std_out);
	xio_set_stderr(std_err);
	xio.default_src = std_in;
	controller_set_primary_source(xio.default_src);
#endif

#ifdef __ARM
	IndicatorLed.setFrequency(100000);
#endif
}

/*
 * controller_init_assertions()
 * controller_test_assertions() - check memory integrity of controller
 */

void controller_init_assertions()
{
	cs.magic_start = MAGICNUM;
	cs.magic_end = MAGICNUM;
}

stat_t controller_test_assertions()
{
	if ((cs.magic_start != MAGICNUM) || (cs.magic_end != MAGICNUM)) return (STAT_CONTROLLER_ASSERTION_FAILURE);
	return (STAT_OK);
}

/*
 * controller_run() - MAIN LOOP - top-level controller
 *
 * The order of the dispatched tasks is very important.
 * Tasks are ordered by increasing dependency (blocking hierarchy).
 * Tasks that are dependent on completion of lower-level tasks must be
 * later in the list than the task(s) they are dependent upon.
 *
 * Tasks must be written as continuations as they will be called repeatedly,
 * and are called even if they are not currently active.
 *
 * The DISPATCH macro calls the function and returns to the controller parent
 * if not finished (STAT_EAGAIN), preventing later routines from running
 * (they remain blocked). Any other condition - OK or ERR - drops through
 * and runs the next routine in the list.
 *
 * A routine that had no action (i.e. is OFF or idle) should return STAT_NOOP
 */

void controller_run()
{
	while (true) {
		_controller_HSM();
	}
}

#define	DISPATCH(func) if (func == STAT_EAGAIN) return;
static void _controller_HSM()
{
//----- Interrupt Service Routines are the highest priority controller functions ----//
//      See hardware.h for a list of ISRs and their priorities.
//
//----- kernel level ISR handlers ----(flags are set in ISRs)------------------------//
												// Order is important:
	DISPATCH(hw_hard_reset_handler());			// 1. handle hard reset requests
	DISPATCH(hw_bootloader_handler());			// 2. handle requests to enter bootloader
	DISPATCH(_shutdown_idler());				// 3. idle in shutdown state
	DISPATCH( poll_switches());					// 4. run a switch polling cycle
	DISPATCH(_limit_switch_handler());			// 5. limit switch has been thrown
    DISPATCH(_interlock_estop_handler());       // 5a. interlock or estop have been thrown
    DISPATCH(_controller_state());				// controller state management

	DISPATCH(_dispatch_control());				// read any control messages prior to executing cycles

	DISPATCH(cm_feedhold_sequencing_callback());// 6a. feedhold state machine runner
	DISPATCH(mp_plan_hold_callback());			// 6b. plan a feedhold from line runtime
	DISPATCH(_system_assertions());				// 8. system integrity assertions

//----- planner hierarchy for gcode and cycles ---------------------------------------//

	DISPATCH(st_motor_power_callback());		// stepper motor power sequencing
//	DISPATCH(switch_debounce_callback());		// debounce switches
	DISPATCH(sr_status_report_callback());		// conditionally send status report
	DISPATCH(qr_queue_report_callback());		// conditionally send queue report
	DISPATCH(rx_report_callback());             // conditionally send rx report

	DISPATCH(cm_arc_cycle_callback());			// arc generation runs as a cycle above lines
	DISPATCH(cm_homing_cycle_callback());		// homing cycle operation (G28.2)
	DISPATCH(cm_probing_cycle_callback());		// probing cycle operation (G38.2)
	DISPATCH(cm_jogging_cycle_callback());		// jog cycle operation
	DISPATCH(cm_deferred_write_callback());		// persist G10 changes when not in machining cycle

//----- command readers and parsers --------------------------------------------------//

	DISPATCH(_sync_to_planner());				// ensure there is at least one free buffer in planning queue
	DISPATCH(_sync_to_tx_buffer());				// sync with TX buffer (pseudo-blocking)
#ifdef __AVR
	DISPATCH(set_baud_callback());				// perform baud rate update (must be after TX sync)
#endif
	DISPATCH(_dispatch_command());				// read and execute next command
	DISPATCH(_normal_idler());					// blink LEDs slowly to show everything is OK
}

/*****************************************************************************************
 * _controller_state() - manage controller connection, startup, and other state changes
 */

static stat_t _controller_state()
{
	if (cs.controller_state == CONTROLLER_CONNECTED) {		// first time through after reset
		cs.controller_state = CONTROLLER_READY;
		cm_request_queue_flush();
        // OOps, we just skipped CONTROLLER_STARTUP. Do we still need it? -r
		rpt_print_system_ready_message();
	}

	return (STAT_OK);
}

/*****************************************************************************************
 * controller_set_connected(bool) - hook for the xio system to tell the controller tha we
 * have/don't have a connection.
 */

void controller_set_connected(bool is_connected) {
    if (is_connected) {
        // we JUST connected
        cs.controller_state = CONTROLLER_CONNECTED;
    } else {
        // we just disconnected from the last device, we'll expext a banner again
        cs.controller_state = CONTROLLER_NOT_CONNECTED;
    }
}


/*****************************************************************************
 * command dispatchers
 * _dispatch_command - entry point for control and data dispatches
 * _dispatch_control - entry point for control-only dispatches
 * _dispatch_kernel - core dispatch routines
 *
 *	Reads next command line and dispatches to relevant parser or action
 */
static stat_t _dispatch_command()
{
	if(cm.estop_state == 0) {
		devflags_t flags = DEV_IS_BOTH;
		if ((cs.bufp = xio_readline(flags, cs.linelen)) != NULL)
        	_dispatch_kernel();
	}
	return (STAT_OK);
}

static stat_t _dispatch_control()
{
	devflags_t flags = DEV_IS_CTRL;
	if ((cs.bufp = xio_readline(flags, cs.linelen)) != NULL)
        _dispatch_kernel();
	return (STAT_OK);
}

static void _dispatch_kernel()
{
	while ((*cs.bufp == SPC) || (*cs.bufp == TAB)) {		// position past any leading whitespace
		cs.bufp++;
	}
	strncpy(cs.saved_buf, cs.bufp, SAVED_BUFFER_LEN-1);		// save input buffer for reporting

	if (*cs.bufp == NUL) {									// blank line - just a CR or the 2nd termination in a CRLF
		if (cs.comm_mode == TEXT_MODE) {
			text_response(STAT_OK, cs.saved_buf);
		}

	// included for AVR diagnostics
	} else if (*cs.bufp == '!') {
        cm_request_feedhold();
	} else if (*cs.bufp == '%') {
        cm_request_queue_flush();
	} else if (*cs.bufp == '~') {
        cm_request_end_hold();

	} else if (*cs.bufp == '{') {							// process as JSON mode
		cs.comm_mode = JSON_MODE;							// switch to JSON mode
		json_parser(cs.bufp);

	} else if (strchr("$?Hh", *cs.bufp) != NULL) {			// process as text mode
		cs.comm_mode = TEXT_MODE;							// switch to text mode
		text_response(text_parser(cs.bufp), cs.saved_buf);

	} else if (cs.comm_mode == TEXT_MODE) {					// anything else must be Gcode
		text_response(gc_gcode_parser(cs.bufp), cs.saved_buf);

	} else {
		strncpy(cs.out_buf, cs.bufp, (USB_LINE_BUFFER_SIZE-11));	// use out_buf as temp; '-11' is buffer for JSON chars
		sprintf((char *)cs.bufp,"{\"gc\":\"%s\"}\n", (char *)cs.out_buf);
		json_parser(cs.bufp);
	}
}

/**** Local Utilities ********************************************************/
/*
 * _shutdown_idler() - blink rapidly and prevent further activity from occurring
 * _normal_idler() - blink Indicator LED slowly to show everything is OK
 *
 *	Shutdown idler flashes indicator LED rapidly to show everything is not OK.
 *	Shutdown idler returns EAGAIN causing the control loop to never advance beyond
 *	this point. It's important that the reset handler is still called so a SW reset
 *	(ctrl-x) or bootloader request can be processed.
 */

static stat_t _shutdown_idler()
{
	if (cm_get_machine_state() != MACHINE_SHUTDOWN) { return (STAT_OK);}

	if (SysTickTimer_getValue() > cs.led_timer) {
		cs.led_timer = SysTickTimer_getValue() + LED_ALARM_TIMER;
		IndicatorLed.toggle();
	}
	return (STAT_EAGAIN);	// EAGAIN prevents any lower-priority actions from running
}

static stat_t _normal_idler()
{
#ifdef __ARM
	/*
	 * S-curve heartbeat code. Uses forward-differencing math from the stepper code.
	 * See plan_line.cpp for explanations.
	 * Here, the "velocity" goes from 0.0 to 1.0, then back.
	 * t0 = 0, t1 = 0, t2 = 0.5, and we'll complete the S in 100 segments.
	 */

	// These are statics, and the assignments will only evaluate once.
	static float indicator_led_value = 0.0;
	static float indicator_led_forward_diff_1 = 50.0 * square(1.0/100.0);
	static float indicator_led_forward_diff_2 = indicator_led_forward_diff_1 * 2.0;


	if (SysTickTimer.getValue() > cs.led_timer) {
		cs.led_timer = SysTickTimer.getValue() + LED_NORMAL_TIMER / 100;

		indicator_led_value += indicator_led_forward_diff_1;
		if (indicator_led_value > 100.0)
			indicator_led_value = 100.0;

		if ((indicator_led_forward_diff_2 > 0.0 && indicator_led_value >= 50.0) || (indicator_led_forward_diff_2 < 0.0 && indicator_led_value <= 50.0)) {
			indicator_led_forward_diff_2 = -indicator_led_forward_diff_2;
		}
		else if (indicator_led_value <= 0.0) {
			indicator_led_value = 0.0;

			// Reset to account for rounding errors
			indicator_led_forward_diff_1 = 50.0 * square(1.0/100.0);
		} else {
			indicator_led_forward_diff_1 += indicator_led_forward_diff_2;
		}

		IndicatorLed = indicator_led_value/100.0;
	}
#endif
#ifdef __AVR
/*
	if (SysTickTimer_getValue() > cs.led_timer) {
		cs.led_timer = SysTickTimer_getValue() + LED_NORMAL_TIMER;
//		IndicatorLed_toggle();
	}
*/
#endif
	return (STAT_OK);
}

/*
 * controller_reset_source() 		 - reset source to default input device (see note)
 * controller_set_primary_source() 	 - set current primary input source
 * controller_set_secondary_source() - set current primary input source
 *
 * Note: Once multiple serial devices are supported reset_source() should be expanded to
 * also set the stdout/stderr console device so the prompt and other messages are sent
 * to the active device.
 */
/*
void controller_reset_source() { controller_set_primary_source(xio.default_src);}
void controller_set_primary_source(uint8_t dev) { xio.primary_src = dev;}
void controller_set_secondary_source(uint8_t dev) { xio.secondary_src = dev;}
*/
/*
 * _sync_to_tx_buffer() - return eagain if TX queue is backed up
 * _sync_to_planner() - return eagain if planner is not ready for a new command
 * _sync_to_time() - return eagain if planner is not ready for a new command
 */

static stat_t _sync_to_tx_buffer()
{
//	if ((xio_get_tx_bufcount_usart(ds[XIO_DEV_USB].x) >= XOFF_TX_LO_WATER_MARK)) {
//		return (STAT_EAGAIN);
//	}
	return (STAT_OK);
}

static stat_t _sync_to_planner()
{
	if (mp_get_planner_buffers_available() < PLANNER_BUFFER_HEADROOM) { // allow up to N planner buffers for this line
		return (STAT_EAGAIN);
	}
	return (STAT_OK);
}
/*
static stat_t _sync_to_time()
{
	if (cs.sync_to_time_time == 0) {		// initial pass
		cs.sync_to_time_time = SysTickTimer_getValue() + 100; //ms
		return (STAT_OK);
	}
	if (SysTickTimer_getValue() < cs.sync_to_time_time) {
		return (STAT_EAGAIN);
	}
	return (STAT_OK);
}
*/

/*
 * _limit_switch_handler() - shut down system if limit switch fired
 */
static stat_t _limit_switch_handler(void)
{
	if (get_limit_switch_thrown() == false) {
        return (STAT_NOOP);
    } else {
        return cm_hard_alarm(STAT_LIMIT_SWITCH_HIT);
    }
}

static stat_t _interlock_estop_handler(void)
{
#ifdef ENABLE_INTERLOCK_AND_ESTOP
	bool report = false;
	if(cm.interlock_state == 0 && read_switch(INTERLOCK_SWITCH_AXIS, INTERLOCK_SWITCH_POSITION) == SW_CLOSED) {
		cm.interlock_state = 1;
		if(cm.gm.spindle_mode != SPINDLE_OFF)
			cm_request_feedhold();
		report = true;
	} else if(cm.interlock_state == 1 && read_switch(INTERLOCK_SWITCH_AXIS, INTERLOCK_SWITCH_POSITION) == SW_OPEN) {
		cm.interlock_state = 0;
		report = true;
	}
	if((cm.estop_state & ESTOP_PRESSED_MASK) == ESTOP_RELEASED && read_switch(ESTOP_SWITCH_AXIS, ESTOP_SWITCH_POSITION) == SW_CLOSED) {
		cm.estop_state = ESTOP_PRESSED | ESTOP_UNACKED | ESTOP_ACTIVE;
		report = true;
		cm_start_estop();
	} else if((cm.estop_state & ESTOP_PRESSED_MASK) == ESTOP_PRESSED && read_switch(ESTOP_SWITCH_AXIS, ESTOP_SWITCH_POSITION) == SW_OPEN) {
		cm.estop_state &= ~ESTOP_PRESSED;
		report = true;
	}
	if(cm.estop_state == ESTOP_ACTIVE) {
        cm.estop_state = 0;
		cm_end_estop();
		report = true;
	}
	if(report)
		sr_request_status_report(SR_REQUEST_IMMEDIATE);
	return (STAT_OK);
#else
	return (STAT_OK);
#endif
}

/*
 * _system_assertions() - check memory integrity and other assertions
 */
#define emergency___everybody_to_get_from_street(a) if((status_code=a) != STAT_OK) return (cm_hard_alarm(status_code));

stat_t _system_assertions()
{
	emergency___everybody_to_get_from_street(config_test_assertions());
	emergency___everybody_to_get_from_street(controller_test_assertions());
	emergency___everybody_to_get_from_street(canonical_machine_test_assertions());
	emergency___everybody_to_get_from_street(planner_test_assertions());
	emergency___everybody_to_get_from_street(stepper_test_assertions());
	emergency___everybody_to_get_from_street(encoder_test_assertions());
	emergency___everybody_to_get_from_street(xio_test_assertions());
	return (STAT_OK);
}
