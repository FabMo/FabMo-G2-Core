/*
 * motate_pin_assignments.h - pin assignments
 * For: /board/gQuintic
 * This file is part of the g2core project
 *
 * Copyright (c) 2013 - 2018 Robert Giseburt
 * Copyright (c) 2013 - 2018 Alden S. Hart Jr.
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

// Board pinout is pulled in after naming, so we can use the naming there.

namespace Motate {

// NOT ALL OF THESE PINS ARE ON ALL PLATFORMS
// Undefined pins will be equivalent to Motate::NullPin, and return 1 for Pin<>::isNull();


pin_number kSerial_RXPinNumber  = -1;  // 0;
pin_number kSerial_TXPinNumber  = -1;  // 1;
pin_number kSerial_RTSPinNumber = -1;  // 2; added later
pin_number kSerial_CTSPinNumber = -1;  // 3; added later

pin_number kSerial0_RXPinNumber  = -1;  // 0;
pin_number kSerial0_TXPinNumber  = -1;  // 1;
pin_number kSerial0_RTSPinNumber = -1;  // 2; added later
pin_number kSerial0_CTSPinNumber = -1;  // 3; added later

pin_number kI2C_SDAPinNumber = -1;  // 5;
pin_number kI2C_SCLPinNumber = -1;  // 6;

pin_number kI2C0_SDAPinNumber = -1;  // not pinned out
pin_number kI2C0_SCLPinNumber = -1;  // not pinned out

pin_number kI2C1_SDAPinNumber = -1;  // 5;
pin_number kI2C1_SCLPinNumber = -1;  // 6;

pin_number kSPI_SCKPinNumber  = -1;  // 7;
pin_number kSPI_MISOPinNumber = -1;  // 8;
pin_number kSPI_MOSIPinNumber = -1;  // 9;

pin_number kSPI0_SCKPinNumber  = -1;  // 7;
pin_number kSPI0_MISOPinNumber = -1;  // 8;
pin_number kSPI0_MOSIPinNumber = -1;  // 9;

pin_number kKinen_SyncPinNumber = -1;  // not pinned out

pin_number kSocket1_SPISlaveSelectPinNumber = -1;  // 10;
pin_number kSocket1_InterruptPinNumber      = -1;  // 11;
pin_number kSocket1_StepPinNumber           = 12;
pin_number kSocket1_DirPinNumber            = 13;
pin_number kSocket1_EnablePinNumber         = -1;  // 14;
pin_number kSocket1_Microstep_0PinNumber    = -1;  // 15;
pin_number kSocket1_Microstep_1PinNumber    = -1;  // 16;
pin_number kSocket1_Microstep_2PinNumber    = -1;  // 17;
pin_number kSocket1_VrefPinNumber           = -1;  // 18;

pin_number kSocket2_SPISlaveSelectPinNumber = -1;  // 19;
pin_number kSocket2_InterruptPinNumber      = -1;  // 20;
pin_number kSocket2_StepPinNumber           = 21;
pin_number kSocket2_DirPinNumber            = 22;
pin_number kSocket2_EnablePinNumber         = -1;  // 23;
pin_number kSocket2_Microstep_0PinNumber    = -1;  // 24;
pin_number kSocket2_Microstep_1PinNumber    = -1;  // 25;
pin_number kSocket2_Microstep_2PinNumber    = -1;  // 26;
pin_number kSocket2_VrefPinNumber           = -1;  // 27;

pin_number kSocket3_SPISlaveSelectPinNumber = -1;  // 28;
pin_number kSocket3_InterruptPinNumber      = -1;  // 29;
pin_number kSocket3_StepPinNumber           = 30;
pin_number kSocket3_DirPinNumber            = 31;
pin_number kSocket3_EnablePinNumber         = -1;  // 32;
pin_number kSocket3_Microstep_0PinNumber    = -1;  // 33;
pin_number kSocket3_Microstep_1PinNumber    = -1;  // 34;
pin_number kSocket3_Microstep_2PinNumber    = -1;  // 35;
pin_number kSocket3_VrefPinNumber           = -1;  // 36;

pin_number kSocket4_SPISlaveSelectPinNumber = -1;  // 37;
pin_number kSocket4_InterruptPinNumber      = -1;  // 38;
pin_number kSocket4_StepPinNumber           = 39;
pin_number kSocket4_DirPinNumber            = 40;
pin_number kSocket4_EnablePinNumber         = -1;  // 41;
pin_number kSocket4_Microstep_0PinNumber    = -1;  // 42;
pin_number kSocket4_Microstep_1PinNumber    = -1;  // 43;
pin_number kSocket4_Microstep_2PinNumber    = -1;  // 44;
pin_number kSocket4_VrefPinNumber           = -1;  // 45;

pin_number kSocket5_SPISlaveSelectPinNumber = -1;  // 46;
pin_number kSocket5_InterruptPinNumber      = -1;  // 47;
pin_number kSocket5_StepPinNumber           = -1;  // 48;
pin_number kSocket5_DirPinNumber            = -1;  // 49;
pin_number kSocket5_EnablePinNumber         = -1;  // 50;
pin_number kSocket5_Microstep_0PinNumber    = -1;  // 51;
pin_number kSocket5_Microstep_1PinNumber    = -1;  // 52;
pin_number kSocket5_Microstep_2PinNumber    = -1;  // 53;
pin_number kSocket5_VrefPinNumber           = -1;  // 54;

pin_number kSocket6_SPISlaveSelectPinNumber = -1;  // 55;
pin_number kSocket6_InterruptPinNumber      = -1;  // 56;
pin_number kSocket6_StepPinNumber           = -1;  // 57;
pin_number kSocket6_DirPinNumber            = -1;  // 58;
pin_number kSocket6_EnablePinNumber         = -1;  // 59;
pin_number kSocket6_Microstep_0PinNumber    = -1;  // 60;
pin_number kSocket6_Microstep_1PinNumber    = -1;  // 61;
pin_number kSocket6_Microstep_2PinNumber    = -1;  // 62;
pin_number kSocket6_VrefPinNumber           = -1;  // 63;

// We also have to define INPUTx_AVAILABLE so we know if we can make the interrupts
pin_number kInput1_PinNumber  = 100;
pin_number kInput2_PinNumber  = 101;
pin_number kInput3_PinNumber  = -1;  // 102;
pin_number kInput4_PinNumber  = -1;  // 103;
pin_number kInput5_PinNumber  = -1;  // 104;
pin_number kInput6_PinNumber  = -1;  // 105;

pin_number kInput7_PinNumber  = -1;  // 106;
pin_number kInput8_PinNumber  = -1;  // 107;
pin_number kInput9_PinNumber  = -1;  // 108;
pin_number kInput10_PinNumber = -1;  // 109;
pin_number kInput11_PinNumber = -1;  // 110;
pin_number kInput12_PinNumber = -1;  // 111;
pin_number kInput13_PinNumber = -1;  // 112;
pin_number kInput14_PinNumber = -1;  // 113;

// START DEBUG PINS - Convenient pins to hijack for hardware debugging
// To reuse a pin for debug change the original pin number to -1
// and uncomment the corresponding debug pin
pin_number kSpindle_EnablePinNumber = -1;  // 112       // Not implemented Spindle ON implemented in g2ref
pin_number kSpindle_DirPinNumber    = -1;  // 113       // Not implemented Spindle ON implemented in g2ref
pin_number kSpindle_PwmPinNumber    = -1;  // 114       // Not implemented (superseded by Extruder1_PWM)
pin_number kSpindle_Pwm2PinNumber   = -1;  // 115       // Not implemented (superseded by Fan1B_PWM)
pin_number kCoolant_EnablePinNumber = -1;  // 116       // Not implemented Spindle ON implemented in g2ref

pin_number kDebug1_PinNumber = -1;  // 112;
pin_number kDebug2_PinNumber = -1;  // 113;
pin_number kDebug3_PinNumber = -1;  // 116; //e Not the out-of-order numbering & 115 missing
pin_number kDebug4_PinNumber = -1;  // 114;
// END DEBUG PINS

pin_number kLED_USBRXPinNumber     = 117; //
pin_number kLED_USBTXPinNumber     = -1;  // 118;
pin_number kSD_CardDetectPinNumber = -1;
pin_number kSD_ChipSelectPinNumber = -1;
pin_number kUSBVBUS_PinNumber      = 119; //
pin_number kInterlock_InPinNumber  = 121;
pin_number kOutputSAFE_PinNumber   = 122;  // SAFE signal
pin_number kLEDPWM_PinNumber       = 123;
pin_number kOutputInterrupt_PinNumber = 124;  // to-host interrupt signal
pin_number kLED_RGBWPixelPinNumber    = -1;  // 125;

// GRBL / gShield compatibility pins -- Due board ONLY

pin_number kGRBL_ResetPinNumber        = -1;
pin_number kGRBL_FeedHoldPinNumber     = -1;
pin_number kGRBL_CycleStartPinNumber   = -1;
pin_number kGRBL_CommonEnablePinNumber = -1;


pin_number kOutput1_PinNumber  = 130;
pin_number kOutput2_PinNumber  = -1;  // 131;
pin_number kOutput3_PinNumber  = -1;  // 132;
pin_number kOutput4_PinNumber  = 133;
pin_number kOutput5_PinNumber  = -1;  // 134;

pin_number kOutput6_PinNumber  = -1;  // 135;
pin_number kOutput7_PinNumber  = -1;  // 136;
pin_number kOutput8_PinNumber  = -1;  // 137;
pin_number kOutput9_PinNumber  = -1;  // 138;
pin_number kOutput10_PinNumber = -1;  // 139;

pin_number kOutput11_PinNumber = -1;  // 140;
pin_number kOutput12_PinNumber = -1;  // 141;
pin_number kOutput13_PinNumber = -1;  // 142;
pin_number kOutput14_PinNumber = -1;  // 143;
pin_number kOutput15_PinNumber = -1;  // 144;
pin_number kOutput16_PinNumber = -1;  // 145;


pin_number kADC1_PinNumber  = -1;  // 150;
pin_number kADC2_PinNumber  = -1;  // 151;
pin_number kADC3_PinNumber  = -1;  // 152;
pin_number kADC4_PinNumber  = -1;  // 153;

pin_number kExternalClock1_PinNumber = -1;  // 160;  // External pins for exporting a clock signal (for Trinamics)

pin_number kServo1_PinNumber = -1;  // 161;
pin_number kServo2_PinNumber = -1;  // 162;
pin_number kServo3_PinNumber = -1;  // 163;


// start next sequence at 164

// blank spots for unassigned pins - all unassigned pins need a unique number (do not re-use numbers)
pin_number kUnassigned83 = 172;
pin_number kUnassigned82 = 173;
pin_number kUnassigned81 = 174;
pin_number kUnassigned80 = 175;
pin_number kUnassigned79 = 176;
pin_number kUnassigned78 = 177;
pin_number kUnassigned77 = 178;
pin_number kUnassigned76 = 179;
pin_number kUnassigned75 = 180;
pin_number kUnassigned74 = 181;
pin_number kUnassigned73 = 182;
pin_number kUnassigned72 = 183;
pin_number kUnassigned71 = 184;
pin_number kUnassigned70 = 185;
pin_number kUnassigned69 = 186;
pin_number kUnassigned68 = 187;
pin_number kUnassigned67 = 188;
pin_number kUnassigned66 = 189;
pin_number kUnassigned65 = 190;
pin_number kUnassigned64 = 191;
pin_number kUnassigned63 = 192;
pin_number kUnassigned62 = 193;
pin_number kUnassigned61 = 194;
pin_number kUnassigned60 = 195;
pin_number kUnassigned59 = 196;
pin_number kUnassigned58 = 197;
pin_number kUnassigned57 = 198;
pin_number kUnassigned56 = 199;
pin_number kUnassigned55 = 200;
pin_number kUnassigned54 = 201;
pin_number kUnassigned53 = 202;
pin_number kUnassigned52 = 203;
pin_number kUnassigned51 = 204;
pin_number kUnassigned50 = 205;
pin_number kUnassigned49 = 206;
pin_number kUnassigned48 = 207;
pin_number kUnassigned47 = 208;
pin_number kUnassigned46 = 209;
pin_number kUnassigned45 = 210;
pin_number kUnassigned44 = 211;
pin_number kUnassigned43 = 212;
pin_number kUnassigned42 = 213;
pin_number kUnassigned41 = 214;
pin_number kUnassigned40 = 215;
pin_number kUnassigned39 = 216;
pin_number kUnassigned38 = 217;
pin_number kUnassigned37 = 218;
pin_number kUnassigned36 = 219;
pin_number kUnassigned35 = 220;
pin_number kUnassigned34 = 221;
pin_number kUnassigned33 = 222;
pin_number kUnassigned32 = 223;
pin_number kUnassigned31 = 224;
pin_number kUnassigned30 = 225;
pin_number kUnassigned29 = 226;
pin_number kUnassigned28 = 227;
pin_number kUnassigned27 = 228;
pin_number kUnassigned26 = 229;
pin_number kUnassigned25 = 230;
pin_number kUnassigned24 = 231;
pin_number kUnassigned23 = 232;
pin_number kUnassigned22 = 233;
pin_number kUnassigned21 = 234;
pin_number kUnassigned20 = 235;
pin_number kUnassigned19 = 236;
pin_number kUnassigned18 = 237;
pin_number kUnassigned17 = 238;
pin_number kUnassigned16 = 239;
pin_number kUnassigned15 = 240;
pin_number kUnassigned14 = 241;
pin_number kUnassigned13 = 242;
pin_number kUnassigned12 = 243;
pin_number kUnassigned11 = 244;
pin_number kUnassigned10 = 245;
pin_number kUnassigned9  = 246;
pin_number kUnassigned8  = 247;
pin_number kUnassigned7  = 248;
pin_number kUnassigned6  = 249;
pin_number kUnassigned5  = 250;
pin_number kUnassigned4  = 251;
pin_number kUnassigned3  = 252;
pin_number kUnassigned2  = 253;
pin_number kUnassigned1  = 254;  // 254 is the max.. Do not exceed this number
}  // namespace Motate

#ifdef MOTATE_BOARD
#define MOTATE_BOARD_PINOUT <MOTATE_BOARD-pinout.h>
#include MOTATE_BOARD_PINOUT
#else
#error Unknown board layout $(MOTATE_BOARD)
// This next include is for IDEs only
#include <sbv700-pinout.h>
#endif

#endif

// motate_pin_assignments_h
