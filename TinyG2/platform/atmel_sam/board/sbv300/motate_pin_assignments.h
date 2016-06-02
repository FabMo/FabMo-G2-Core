/*
 * http://tinkerin.gs/
 *
 * Copyright (c) 2013 Robert Giseburt
 * Copyright (c) 2013 Alden S. Hart Jr.
 *
 * This file is part of the Motate Library.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
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
 *
 */

#ifndef motate_pin_assignments_h
#define motate_pin_assignments_h

#include <MotateTimers.h>

namespace Motate {

	_MAKE_MOTATE_PORT32(A, 'A');
	_MAKE_MOTATE_PORT32(B, 'B');
	_MAKE_MOTATE_PORT32(C, 'C');
	_MAKE_MOTATE_PORT32(D, 'D');
    
	// STEP Pins
	_MAKE_MOTATE_PIN(0, C, 'C', 28);	// XSTEP
	_MAKE_MOTATE_PIN(1, C, 'C', 25);	// YSTEP
	_MAKE_MOTATE_PIN(2, C, 'C', 26);	// ZSTEP
	_MAKE_MOTATE_PIN(3, C, 'C', 23);	// ASTEP
	_MAKE_MOTATE_PIN(4, C, 'C', 24);	// BSTEP
	_MAKE_MOTATE_PIN(5, B, 'B', 1);	    // CSTEP
	
	// DIR Pins
	_MAKE_MOTATE_PIN(6, B, 'B', 23);	// XDIR
	_MAKE_MOTATE_PIN(7, B, 'B', 24);	// YDIR
	_MAKE_MOTATE_PIN(8, B, 'B', 14);	// ZDIR
	_MAKE_MOTATE_PIN(9, B, 'B', 22);	// ADIR
	_MAKE_MOTATE_PIN(10, C, 'C', 27);	// BDIR
	_MAKE_MOTATE_PIN(11, B, 'B', 0);	// CDIR
	
	// LEDS
	_MAKE_MOTATE_PIN(12, C, 'C', 9);	// USB TXLED
	_MAKE_MOTATE_PIN(13, B, 'B', 27);	// USB RXLED
	
	//  SPINDLE OUTPUTS
	_MAKE_MOTATE_PIN(14, A, 'A', 0);	// SPINDLE ENABLE (OUTPUT 1)
	_MAKE_MOTATE_PIN(15, D, 'D', 7);	// SPINDLE "ALLOW" (OUTPUT 4)
	
	// INPUTS
	_MAKE_MOTATE_PIN( 16, B, 'B', 4 );
	_MAKE_MOTATE_PIN( 17, B, 'B', 5 );
	_MAKE_MOTATE_PIN( 18, C, 'C', 10 );
	_MAKE_MOTATE_PIN( 19, B, 'B', 3 );
	_MAKE_MOTATE_PIN( 20, B, 'B', 8 );
	_MAKE_MOTATE_PIN( 21, B, 'B', 9 );
	_MAKE_MOTATE_PIN( 22, B, 'B', 6 );
	_MAKE_MOTATE_PIN( 23, B, 'B', 7 );
	_MAKE_MOTATE_PIN( 24, D, 'D', 2 );
	_MAKE_MOTATE_PIN( 25, A, 'A', 9 );
	_MAKE_MOTATE_PIN( 26, D, 'D', 1 );
	_MAKE_MOTATE_PIN( 27, A, 'A', 17 );

	// OUTPUTS
	// (NONE YET)
	_MAKE_MOTATE_PIN( 28, D, 'D', 8 );
	
	/*

	POUT1		    J2-pin7 	 PA0		<kSpindle1_EnablePinNumber>  	    (M3: on, M5: off)
POUT2		    J2-pin8	    PD9		<kSpindle2_EnablePinNumber>       (not used)
POUT3		    J2-pin9	    PD8		<kOutput_3>								 (M64p3: on, M65p3: off)
POUT4		    J2-pin10 	 PD7		<kSpindle_DirPinNumber>     	    (M3 or M4: on, M5: off)
POUT5		    J2-pin11	 PD6		<kOutput_5>								 (M64p5: on, M65p5: off)
POUT6		    J2-pin12	 PD5		<kOutput_6>								 (M64p6: on, M65p6: off)
POUT7		    J2-pin13	 PD4		<kOutput_7>								 (M64p7: on, M65p7: off)
POUT8		    J2-pin14	 PD3		<kOutput_8>								 (M64p8: on, M65p8: off)
POUT9			 J2-pin3     PA8     <kOutput_9>								 (not used)
POUT10		 J2-pin4	    PA7		<kOutput_10>							 (M64p10: on, M65p10: off)
POUT11		 J2-pin5	    PA5		<kOutput_11>							 (M64p11: on, M65p11: off)
POUT12		 J2-pin6	    PA1		<kOutput_12>							 (M64p12: on, M65p12: off)

POUT13		 J6-pin8     PA15    <kCoolantFlood_EnablePinNumber>   (M64p13: on, M65p13: off)  
POUT14		 J6-pin9     PA14    <kCoolantMist_EnablePinNumber>    (M64p14: on, M65p14: off)

PPWM0		 	 J6-pin11    PA12    <kSpindle_PwmPinNumber> 	       (M3 or M4: bldc esc on, M5: off, S: rpm)
PPWM1		 	 J6-pin12    PA13    <kSpindle_Pwm2PinNumber> 		    (not used)


*/
} // namespace Motate

#ifdef MOTATE_BOARD
#define MOTATE_BOARD_PINOUT < MOTATE_BOARD-pinout.h >
#include MOTATE_BOARD_PINOUT
#else
#error Unknown board layout
#endif

#endif

// motate_pin_assignments_h
