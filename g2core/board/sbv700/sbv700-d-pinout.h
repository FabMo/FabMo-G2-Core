/*
 * sbv700-c-pinout.h - board pinout specification
 * This file is part of the g2core project
 *
 * Copyright (c) 2017 Robert Giseburt
 * Copyright (c) 2017 Alden S. Hart Jr.
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

#ifndef sbv700_d_pinout_h
#define sbv700_d_pinout_h

/*
 * USAGE NOTES
 *
 * Read this first:
 * https://github.com/synthetos/g2/wiki/Adding-and-Revising-Boards
 *
 *  USAGE:
 *
 *  This file is lays out all the pin capabilities of the SAM3X8C organized by pin number.
 *  Each pin has its associated functions listed at the bottom of the file, and is essentially
 *  immutable for each processor.
 *
 *  To use, assign Motate pin numbers to the first value in the _MAKE_MOTATE_PIN() macro.
 *  ALL PINS MUST BE ASSIGNED A NUMBER, even if they are not used. There will NOT be a
 *  code-size or speed penalty for unused pins, but the WILL be a compiler-failure for
 *  unassigned pins. This new restriction allows for simplification of linkages deep in
 *  Motate.
 */
/*  See motate_pin_assignments.h for pin names to be used int he rest of the G2 code.
 *  EXAMPLES:
 *
 *  *** Vanilla pin example ***
 *
 *      _MAKE_MOTATE_PIN(4, A, 'A', 27);	// SPI0_SCKPinNumber
 *
 *      This assigns Motate pin 4 to Port A, pin 27 (A27)
 *      Look in motate_pin_assignments.h to see that this is kSPI_SCKPinNumber
 *
 *  ** Other pin functions ***
 *
 *      Please look in <Motate>/platform/atmel_sam/motate_chip_pin_functions.h
 */


#include <MotateTimers.h>

// We don't have all of the inputs, so we have to indicate as much:
#define INPUT1_AVAILABLE 0
#define INPUT2_AVAILABLE 0
#define INPUT3_AVAILABLE 0
#define INPUT4_AVAILABLE 0
#define INPUT5_AVAILABLE 0
#define INPUT6_AVAILABLE 0
#define INPUT7_AVAILABLE 0
#define INPUT8_AVAILABLE 0
#define INPUT9_AVAILABLE 0
#define INPUT10_AVAILABLE 0
#define INPUT11_AVAILABLE 0
#define INPUT12_AVAILABLE 0
#define INPUT13_AVAILABLE 0

#define XIO_HAS_USB 1
#define XIO_HAS_UART 0
#define XIO_HAS_SPI 0
#define XIO_HAS_I2C 0

#define TEMPERATURE_OUTPUT_ON 0 // needs more testing

// Some pins, if the PWM capability is turned on, it will cause timer conflicts.
// So we have to explicitly enable them as PWM pins.
// Generated with:
// perl -e 'for($i=1;$i<14;$i++) { print "#define OUTPUT${i}_PWM 0\n";}'
//#define OUTPUT1_PWM 1   // TC 0,1 - Fet 1
//#define OUTPUT2_PWM 1   // PWM 0,1 - Fet 2
#define OUTPUT1_PWM 0   // Used by EX1
#define OUTPUT2_PWM 0   // Used by EX2
#define OUTPUT3_PWM 0   // TC 1,0 - Fan 1
#define OUTPUT4_PWM 0   // TC 1,1 - Fan 2
#define OUTPUT5_PWM 0   // TC 2,0 - Fan 3
#define OUTPUT6_PWM 0   // PWM 1,0
#define OUTPUT7_PWM 0   // PWM 0,3
#define OUTPUT8_PWM 0   // PWM 0,2
#define OUTPUT9_PWM 0   // PWM 0,2
#define OUTPUT10_PWM 0  // PWM 1,2
#define OUTPUT11_PWM 0  // Used by Heatbed
#define OUTPUT12_PWM 0  // Unused
#define OUTPUT13_PWM 0  // Unused

namespace Motate {

// Unused:
//
//
//
//
//

_MAKE_MOTATE_PIN(kLED_RGBWPixelPinNumber,          'A',  0); //
_MAKE_MOTATE_PIN(kOutput1_PinNumber,               'A',  1); // TC  0,1
_MAKE_MOTATE_PIN(kOutput2_PinNumber,               'A',  2); // PWM 0,1
_MAKE_MOTATE_PIN(kUnassigned1,                     'A',  3); //
_MAKE_MOTATE_PIN(kUnassigned2,                     'A',  4); //
_MAKE_MOTATE_PIN(kOutput11_PinNumber,              'A',  5); // PWM 1,3
_MAKE_MOTATE_PIN(kUnassigned3,                     'A',  6); // CPU_CLK
_MAKE_MOTATE_PIN(kOutput7_PinNumber,               'A',  7); // PWM 0,3
_MAKE_MOTATE_PIN(kUnassigned4,                     'A',  8); //
_MAKE_MOTATE_PIN(kUnassigned5,                     'A',  9); //
_MAKE_MOTATE_PIN(kUnassigned6,                     'A', 10); //
_MAKE_MOTATE_PIN(kUnassigned7,                     'A', 11); //
_MAKE_MOTATE_PIN(kOutput6_PinNumber,               'A', 12); // PWM 1,0
_MAKE_MOTATE_PIN(kOutput8_PinNumber,               'A', 13); // PWM 0,2
_MAKE_MOTATE_PIN(kSocket1_StepPinNumber,           'A', 14); //
_MAKE_MOTATE_PIN(kOutput3_PinNumber,               'A', 15); // TC 1,0
_MAKE_MOTATE_PIN(kOutput4_PinNumber,               'A', 16); // TC 1,1
_MAKE_MOTATE_PIN(kUnassigned8,                     'A', 17); // AFEC0,6
_MAKE_MOTATE_PIN(kUnassigned9,                     'A', 18); // AFEC0,7
_MAKE_MOTATE_PIN(kUnassigned10,                    'A', 19); // AFEC0,8
_MAKE_MOTATE_PIN(kUnassigned11,                    'A', 20); // AFEC0,9
_MAKE_MOTATE_PIN(kUnassigned12,                    'A', 21);
_MAKE_MOTATE_PIN(kUnassigned13,                    'A', 22); //
_MAKE_MOTATE_PIN(kLED_USBRXPinNumber,              'A', 23); // PWM 1,2 This is so that LED0 is heartbeat
_MAKE_MOTATE_PIN(kUnassigned14,                    'A', 24); //
_MAKE_MOTATE_PIN(kSocket2_DirPinNumber,            'A', 25); //
_MAKE_MOTATE_PIN(kOutput5_PinNumber,               'A', 26); // TC 2,0
_MAKE_MOTATE_PIN(kSocket3_DirPinNumber,            'A', 27); // On Timer 2!
_MAKE_MOTATE_PIN(kUnassigned15,                    'A', 28); // DIAG1
_MAKE_MOTATE_PIN(kUnassigned16,                    'A', 29); // NO PHYSICAL PIN
_MAKE_MOTATE_PIN(kInput1_PinNumber,                'A', 30); //
_MAKE_MOTATE_PIN(kInput4_PinNumber,                'A', 31); //

_MAKE_MOTATE_PIN(kUnassigned17,                    'B',  0); //
_MAKE_MOTATE_PIN(kUnassigned18,                    'B',  1); // AFEC1,0
_MAKE_MOTATE_PIN(kUnassigned19,                    'B',  2); //
_MAKE_MOTATE_PIN(kOutputSAFE_PinNumber,            'B',  3); //
_MAKE_MOTATE_PIN(kLED_USBTXPinNumber,              'B',  4);
_MAKE_MOTATE_PIN(kUnassigned20,                    'B', 13); // LED_1 (Heartbeat) - PWM2
_MAKE_MOTATE_PIN(kUnassigned21,                    'B', 14); // NOT CONNECTED

_MAKE_MOTATE_PIN(kUnassigned22,                    'C',  0); //
_MAKE_MOTATE_PIN(kUnassigned23,                    'C',  1); //
_MAKE_MOTATE_PIN(kUnassigned24,                    'C',  2); //
_MAKE_MOTATE_PIN(kUnassigned25,                    'C',  3); //

_MAKE_MOTATE_PIN(kUnassigned26,                    'C',  5); //
_MAKE_MOTATE_PIN(kUnassigned27,                    'C',  6); //

_MAKE_MOTATE_PIN(kUnassigned28,                    'C',  8); //
_MAKE_MOTATE_PIN(kUSBVBUS_PinNumber,               'C',  9); // USB_VBUS

_MAKE_MOTATE_PIN(kUnassigned29,                    'C', 11); //
_MAKE_MOTATE_PIN(kUnassigned30,                    'C', 12); //
_MAKE_MOTATE_PIN(kUnassigned31,                    'C', 13); //

_MAKE_MOTATE_PIN(kUnassigned32,                    'C', 15); //

_MAKE_MOTATE_PIN(kUnassigned33,                    'C', 18); //
_MAKE_MOTATE_PIN(kUnassigned34,                    'C', 19); //
_MAKE_MOTATE_PIN(kUnassigned35,                    'C', 20); //
_MAKE_MOTATE_PIN(kUnassigned36,                    'C', 21); //
_MAKE_MOTATE_PIN(kUnassigned37,                    'C', 22); //
_MAKE_MOTATE_PIN(kUnassigned38,                    'C', 23); //
_MAKE_MOTATE_PIN(kUnassigned39,                    'C', 24); //
_MAKE_MOTATE_PIN(kUnassigned40,                    'C', 25); //
_MAKE_MOTATE_PIN(kUnassigned41,                    'C', 26); //
_MAKE_MOTATE_PIN(kInput12_PinNumber,               'C', 27); //
_MAKE_MOTATE_PIN(kInput11_PinNumber,               'C', 28); //
_MAKE_MOTATE_PIN(kInput10_PinNumber,               'C', 29); //
_MAKE_MOTATE_PIN(kInput9_PinNumber,                'C', 30); //
_MAKE_MOTATE_PIN(kInput8_PinNumber,                'C', 31); //

_MAKE_MOTATE_PIN(kUnassigned44,                    'D',  0);
_MAKE_MOTATE_PIN(kSocket6_StepPinNumber,           'D',  1); // Switched with Input9
_MAKE_MOTATE_PIN(kSocket6_DirPinNumber,            'D',  2); // Switched with Input10
_MAKE_MOTATE_PIN(kUnassigned47,                    'D',  3); // Switched with Input8
_MAKE_MOTATE_PIN(kInput7_PinNumber,                'D',  4); //
_MAKE_MOTATE_PIN(kInput6_PinNumber,                'D',  5); //
_MAKE_MOTATE_PIN(kInput5_PinNumber,                'D',  6); //
_MAKE_MOTATE_PIN(kInput3_PinNumber,                'D',  7); //
_MAKE_MOTATE_PIN(kUnassigned48,                    'D',  8); // Switched with Input2
_MAKE_MOTATE_PIN(kUnassigned49,                    'D',  9); // DIAG0
_MAKE_MOTATE_PIN(kInput2_PinNumber,                'D', 10); //
_MAKE_MOTATE_PIN(kSocket5_StepPinNumber,           'D', 11); //
_MAKE_MOTATE_PIN(kUnassigned50,                    'D', 12); //
_MAKE_MOTATE_PIN(kSocket5_DirPinNumber,            'D', 13); //
_MAKE_MOTATE_PIN(kUnassigned51,                    'D', 14); //
_MAKE_MOTATE_PIN(kUnassigned52,                    'D', 15); //
_MAKE_MOTATE_PIN(kSocket4_StepPinNumber,           'D', 16); //
_MAKE_MOTATE_PIN(kSocket4_DirPinNumber,            'D', 17); //
_MAKE_MOTATE_PIN(kSocket3_StepPinNumber,           'D', 18); //
_MAKE_MOTATE_PIN(kSocket1_DirPinNumber,            'D', 19); //
_MAKE_MOTATE_PIN(kUnassigned53,                    'D', 20); //
_MAKE_MOTATE_PIN(kUnassigned54,                    'D', 21); //
_MAKE_MOTATE_PIN(kUnassigned55,                    'D', 22); //
_MAKE_MOTATE_PIN(kUnassigned56,                    'D', 23); // NO PHYSICAL PIN
_MAKE_MOTATE_PIN(kSocket2_StepPinNumber,           'D', 24); //
_MAKE_MOTATE_PIN(kUnassigned57,                    'D', 25); //
_MAKE_MOTATE_PIN(kOutput9_PinNumber,               'D', 26); // PWM 2
_MAKE_MOTATE_PIN(kOutput10_PinNumber,              'D', 27); //
_MAKE_MOTATE_PIN(kUnassigned58,                    'D', 28); //
_MAKE_MOTATE_PIN(kOutput12_PinNumber,              'D', 29); // NO PHYSICAL PIN
_MAKE_MOTATE_PIN(kOutput13_PinNumber,              'D', 30); // AFEC0,0 - was INTERRUPT_OUT
_MAKE_MOTATE_PIN(kUnassigned62,                    'D', 31); //

_MAKE_MOTATE_PIN(kUnassigned63,                    'E',  0); //
_MAKE_MOTATE_PIN(kUnassigned64,                    'E',  3); //
_MAKE_MOTATE_PIN(kUnassigned65,                    'E',  4); //
_MAKE_MOTATE_PIN(kUnassigned66,                    'E',  5); //
}  // namespace Motate

// We then allow each chip-type to have it's own function definitions
// that will refer to these pin assignments.
#include "motate_chip_pin_functions.h"

#endif

// sbv700_a_pinout_h
