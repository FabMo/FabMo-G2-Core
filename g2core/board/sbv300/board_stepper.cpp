/*
 * board_stepper.cpp - board-specific code for stepper.cpp
 * For: /board/sbv300
 * This file is part of the g2core project
 *
 * Copyright (c) 2016 - 2018 Alden S. Hart, Jr.
 * Copyright (c) 2016 - 2018 Robert Giseburt
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

#include "board_stepper.h"

// These are identical to board_stepper.h, except for the word "extern", and they have the initialization parameters
StepDirStepper<Motate::kSocket1_StepPinNumber,
               Motate::kSocket1_DirPinNumber,
               Motate::kSocket1_EnablePinNumber,
               Motate::kSocket1_Microstep_0PinNumber,
               Motate::kSocket1_Microstep_1PinNumber,
               Motate::kSocket1_Microstep_2PinNumber,
               Motate::kSocket1_VrefPinNumber>
    motor_1{M1_STEP_POLARITY, M1_ENABLE_POLARITY};

StepDirStepper<Motate::kSocket2_StepPinNumber,
               Motate::kSocket2_DirPinNumber,
               Motate::kSocket2_EnablePinNumber,
               Motate::kSocket2_Microstep_0PinNumber,
               Motate::kSocket2_Microstep_1PinNumber,
               Motate::kSocket2_Microstep_2PinNumber,
               Motate::kSocket2_VrefPinNumber>
    motor_2{M2_STEP_POLARITY, M2_ENABLE_POLARITY};

StepDirStepper<Motate::kSocket3_StepPinNumber,
               Motate::kSocket3_DirPinNumber,
               Motate::kSocket3_EnablePinNumber,
               Motate::kSocket3_Microstep_0PinNumber,
               Motate::kSocket3_Microstep_1PinNumber,
               Motate::kSocket3_Microstep_2PinNumber,
               Motate::kSocket3_VrefPinNumber>
    motor_3{M3_STEP_POLARITY, M3_ENABLE_POLARITY};

StepDirStepper<Motate::kSocket4_StepPinNumber,
               Motate::kSocket4_DirPinNumber,
               Motate::kSocket4_EnablePinNumber,
               Motate::kSocket4_Microstep_0PinNumber,
               Motate::kSocket4_Microstep_1PinNumber,
               Motate::kSocket4_Microstep_2PinNumber,
               Motate::kSocket4_VrefPinNumber>
    motor_4{M4_STEP_POLARITY, M4_ENABLE_POLARITY};

StepDirStepper<Motate::kSocket5_StepPinNumber,
               Motate::kSocket5_DirPinNumber,
               Motate::kSocket5_EnablePinNumber,
               Motate::kSocket5_Microstep_0PinNumber,
               Motate::kSocket5_Microstep_1PinNumber,
               Motate::kSocket5_Microstep_2PinNumber,
               Motate::kSocket5_VrefPinNumber>
    motor_5 {M5_STEP_POLARITY, M5_ENABLE_POLARITY};


#if HAS_LASER
// For laser mode, motor_6 is defined as a laser tool in hardware.cpp
// reference it here
extern LaserTool_used_t motor_6;

// Motors array includes the laser tool as motor_6
Stepper* const Motors[MOTORS] = {&motor_1, &motor_2, &motor_3, &motor_4, &motor_5, &motor_6};
#else

// For non-laser mode, motor_6 is a regular stepper
StepDirStepper<Motate::kSocket6_StepPinNumber,
               Motate::kSocket6_DirPinNumber,
               Motate::kSocket6_EnablePinNumber,
               Motate::kSocket6_Microstep_0PinNumber,
               Motate::kSocket6_Microstep_1PinNumber,
               Motate::kSocket6_Microstep_2PinNumber,
               Motate::kSocket6_VrefPinNumber>
    motor_6 {M6_STEP_POLARITY, M6_ENABLE_POLARITY};

Stepper* const Motors[MOTORS] = {&motor_1, &motor_2, &motor_3, &motor_4, &motor_5, &motor_6};
#endif

void board_stepper_init() {
    for (uint8_t motor = 0; motor < MOTORS; motor++) { Motors[motor]->init(); }
}