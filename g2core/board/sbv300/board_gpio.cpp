/*
 * gpio.cpp - digital IO handling functions
 * This file is part of the g2core project
 *
 * Copyright (c) 2015 - 2107 Alden S. Hart, Jr.
 * Copyright (c) 2015 - 2017 Robert Giseburt
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
 */
/* Switch Modes
 *
 *  The switches are considered to be homing switches when cycle_state is
 *  CYCLE_HOMING. At all other times they are treated as limit switches:
 *    - Hitting a homing switch puts the current move into feedhold
 *    - Hitting a limit switch causes the machine to shut down and go into lockdown until reset
 *
 *  The normally open switch modes (NO) trigger an interrupt on the falling edge
 *  and lockout subsequent interrupts for the defined lockout period. This approach
 *  beats doing debouncing as an integration as switches fire immediately.
 *
 *  The normally closed switch modes (NC) trigger an interrupt on the rising edge
 *  and lockout subsequent interrupts for the defined lockout period. Ditto on the method.
 */

#include "../../g2core.h"  // #1
#include "config.h"  // #2
#include "gpio.h"
#include "hardware.h"
#include "canonical_machine.h"

#include "text_parser.h"
#include "controller.h"
#include "util.h"
#include "report.h"
#include "xio.h"

#include "MotateTimers.h"

/**** Setup Actual Objects ****/
//#### added more outputs for PWM & LASER

gpioDigitalInputPin<IRQPin<Motate::kInput1_PinNumber>>  din1  {DI1_ENABLED,  DI1_POLARITY,  1, DI1_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput2_PinNumber>>  din2  {DI2_ENABLED,  DI2_POLARITY,  2, DI2_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput3_PinNumber>>  din3  {DI3_ENABLED,  DI3_POLARITY,  3, DI3_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput4_PinNumber>>  din4  {DI4_ENABLED,  DI4_POLARITY,  4, DI4_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput5_PinNumber>>  din5  {DI5_ENABLED,  DI5_POLARITY,  5, DI5_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput6_PinNumber>>  din6  {DI6_ENABLED,  DI6_POLARITY,  6, DI6_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput7_PinNumber>>  din7  {DI7_ENABLED,  DI7_POLARITY,  7, DI7_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput8_PinNumber>>  din8  {DI8_ENABLED,  DI8_POLARITY,  8, DI8_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput9_PinNumber>>  din9  {DI9_ENABLED,  DI9_POLARITY,  9, DI9_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput10_PinNumber>> din10 {DI10_ENABLED, DI10_POLARITY, 10, DI10_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput11_PinNumber>> din11 {DI11_ENABLED, DI11_POLARITY, 11, DI11_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput12_PinNumber>> din12 {DI12_ENABLED, DI12_POLARITY, 12, DI12_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput13_PinNumber>> din13 {DI13_ENABLED, DI13_POLARITY, 13, DI13_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput14_PinNumber>> din14 {DI14_ENABLED, DI14_POLARITY, 14, DI14_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput15_PinNumber>> din15 {DI15_ENABLED, DI15_POLARITY, 15, DI15_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput16_PinNumber>> din16 {DI16_ENABLED, DI16_POLARITY, 16, DI16_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput17_PinNumber>> din17 {DI17_ENABLED, DI17_POLARITY, 17, DI17_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};
gpioDigitalInputPin<IRQPin<Motate::kInput18_PinNumber>> din18 {DI18_ENABLED, DI18_POLARITY, 18, DI18_EXTERNAL_NUMBER, Motate::kPinInterruptOnChange|Motate::kPinInterruptPriorityHigh};

gpioDigitalOutputPin<OutputType<OUTPUT1_PWM,  Motate::kOutput1_PinNumber>>  dout1  { DO1_ENABLED,  DO1_POLARITY,  DO1_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT2_PWM,  Motate::kOutput2_PinNumber>>  dout2  { DO2_ENABLED,  DO2_POLARITY,  DO2_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT3_PWM,  Motate::kOutput3_PinNumber>>  dout3  { DO3_ENABLED,  DO3_POLARITY,  DO3_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT4_PWM,  Motate::kOutput4_PinNumber>>  dout4  { DO4_ENABLED,  DO4_POLARITY,  DO4_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT5_PWM,  Motate::kOutput5_PinNumber>>  dout5  { DO5_ENABLED,  DO5_POLARITY,  DO5_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT6_PWM,  Motate::kOutput6_PinNumber>>  dout6  { DO6_ENABLED,  DO6_POLARITY,  DO6_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT7_PWM,  Motate::kOutput7_PinNumber>>  dout7  { DO7_ENABLED,  DO7_POLARITY,  DO7_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT8_PWM,  Motate::kOutput8_PinNumber>>  dout8  { DO8_ENABLED,  DO8_POLARITY,  DO8_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT9_PWM,  Motate::kOutput9_PinNumber>>  dout9  { DO9_ENABLED,  DO9_POLARITY,  DO9_EXTERNAL_NUMBER,  (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT10_PWM, Motate::kOutput10_PinNumber>> dout10 { DO10_ENABLED, DO10_POLARITY, DO10_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT11_PWM, Motate::kOutput11_PinNumber>> dout11 { DO11_ENABLED, DO11_POLARITY, DO11_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT12_PWM, Motate::kOutput12_PinNumber>> dout12 { DO12_ENABLED, DO12_POLARITY, DO12_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT13_PWM, Motate::kOutput13_PinNumber>> dout13 { DO13_ENABLED, DO13_POLARITY, DO13_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT14_PWM, Motate::kOutput14_PinNumber>> dout14 { DO14_ENABLED, DO14_POLARITY, DO14_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT15_PWM, Motate::kOutput15_PinNumber>> dout15 { DO15_ENABLED, DO15_POLARITY, DO15_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT16_PWM, Motate::kOutput16_PinNumber>> dout16 { DO16_ENABLED, DO16_POLARITY, DO16_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT17_PWM, Motate::kOutput17_PinNumber>> dout17 { DO17_ENABLED, DO17_POLARITY, DO17_EXTERNAL_NUMBER, (uint32_t)200000 };
gpioDigitalOutputPin<OutputType<OUTPUT18_PWM, Motate::kOutput18_PinNumber>> dout18 { DO18_ENABLED, DO18_POLARITY, DO18_EXTERNAL_NUMBER, (uint32_t)200000 };

// Simple ADC object creation (following gQuintic pattern):
gpioAnalogInputPin<ADCPin<Motate::kADC1_PinNumber>> ai1 {AI1_ENABLED, gpioAnalogInput::AIN_TYPE_INTERNAL, 1, AI1_EXTERNAL_NUMBER};
gpioAnalogInputPin<ADCPin<Motate::kADC2_PinNumber>> ai2 {AI2_ENABLED, gpioAnalogInput::AIN_TYPE_INTERNAL, 2, AI2_EXTERNAL_NUMBER};
gpioAnalogInputPin<ADCPin<Motate::kADC3_PinNumber>> ai3 {AI3_ENABLED, gpioAnalogInput::AIN_TYPE_INTERNAL, 3, AI3_EXTERNAL_NUMBER};
gpioAnalogInputPin<ADCPin<Motate::kADC4_PinNumber>> ai4 {AI4_ENABLED, gpioAnalogInput::AIN_TYPE_INTERNAL, 4, AI4_EXTERNAL_NUMBER};

/**** Setup Arrays - these are extern and MUST match the board_gpio.h ****/

gpioDigitalInput*  const d_in[] = {&din1, &din2, &din3, &din4, &din5, &din6, &din7, &din8, &din9, &din10, &din11, &din12,&din13, &din14, &din15, &din16, &din17, &din18};
gpioDigitalOutput* const d_out[] = {&dout1, &dout2, &dout3, &dout4, &dout5, &dout6, &dout7, &dout8, &dout9, &dout10, &dout11, &dout12, &dout13, &dout14, &dout15, &dout16, &dout17, &dout18};

gpioAnalogInput* const a_in[] = {&ai1, &ai2, &ai3, &ai4};

/************************************************************************************
 **** CODE **************************************************************************
 ************************************************************************************/
// Register a SysTick event to call start_sampling every ain_sample_freq ms
const int16_t ain_sample_freq = 200;  // Sample every 200ms
int16_t ain_sample_counter = ain_sample_freq;
Motate::SysTickEvent ain_tick_event{
    [] {
        if (!--ain_sample_counter) {
            ai1.startSampling();
            ai2.startSampling();
            ai3.startSampling();
            ai4.startSampling();
            
            ain_sample_counter = ain_sample_freq;
        }
    },
    nullptr
};

/*
 * gpio_reset() - reset inputs and outputs (no initialization)
 */

void outputs_reset(void) {
    // nothing to do
}

void inputs_reset(void) {
    SysTickTimer.registerEvent(&ain_tick_event);
}