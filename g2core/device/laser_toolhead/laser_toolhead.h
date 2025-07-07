/*
 * laser_toolhead.h - toolhead driver for a laser, controlled by spindle commands
 * This file is part of the g2core project
 *
 * Copyright (c) 2020 Robert Giseburt
 * Copyright (c) 2020 Alden S. Hart, Jr.
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

#ifndef LASER_TOOLHEAD_H_ONCE
#define LASER_TOOLHEAD_H_ONCE

template <typename KinematicsParent, Motate::pin_number fire_num>
class LaserTool; // Need to forward declare since stepper.h may (eventually) include this file

#include "util.h"
#include "spindle.h"
#include "stepper.h" // for Stepper and st_request_load_move
#include "safety_manager.h" // for safety_manager

/* A few notes:
 *
 * This module drives a laser output based on the spindle controls.
 *
 * Laser ON/OFF (NOT fire, just "is active") is on the `enable_output` pin,
 * and actual fire/pulse is on the `fire` pin.
 *
 */


// class declaration
// note implementation is after
template <typename KinematicsParent, Motate::pin_number fire_num>
class LaserTool : public ToolHead, public Stepper, public KinematicsParent {
    spDirection direction;        // direction
    float speed;                  // S in RPM

    float speed_override_factor = 1;
    bool speed_override_enable = true;

    float speed_min;              // minimum settable spindle speed
    float speed_max;              // maximum settable spindle speed

    bool paused;                  // true if paused, false is not

    PWMOutputPin<fire_num> fire;

    uint8_t enable_output_num;
    gpioDigitalOutput *enable_output = nullptr;

    // For "Stepper" enabled control
    bool enabled = false;
    uint8_t laser_motor;

    float laser_step_position;
    int16_t pulse_tick_counter = 0;
    int16_t ticks_per_pulse;
    int16_t next_ticks_per_pulse;
    int16_t pulse_duration_us;

    uint32_t raw_fire_duty_cycle = 0;

    float min_s;
    float max_s;
    float min_ppm;
    float max_ppm;

    void complete_change();

    enum LaserMode {
        LASER_MODE_STATIC,     // M3/M4 static power
        LASER_MODE_MOTION      // Motion-synchronized power
    };
    LaserMode current_mode = LASER_MODE_STATIC;

   public:

// ToolHead functions ----
    // constructor - provide it with the default output pins - 0 means no pin
    LaserTool(const uint8_t enable_pin_number, const uint8_t laser_motor);

    // ToolHead overrides
    void init() override;

    void pause() override;          // soft-stop the toolhead (usually for a feedhold) - retain all state for resume
    void resume() override;         // resume from the pause - return STAT_EAGAIN if it's not yet ready
    bool ready_to_resume() override;  // return true if paused and resume would not result in an error
    // bool busy() override;             // return true if motion should continue waiting for this toolhead

    // the result of an S word
    // override this to return false - "don't add a command to the buffer"
    // bool set_speed(float) override;
    float get_speed() override;

    // set the override value for spindle speed
    bool set_override(float override) override;
    float get_override() override;

    // enable or disable the override
    bool set_override_enable(bool override_enable) override;
    bool get_override_enable() override;

    // the result of an M3/M4/M5
    // override this to return false - "don't add a command to the buffer"
    // bool set_direction(spDirection) override;
    spDirection get_direction() override;

    void stop() override;

    // called from the loader right before a move, with the gcode model to use
    void engage(const GCodeState_t &gm) override;

    bool is_on() override;  // return if the current direction is anything but OFF, **even if paused**

    bool set_pwm_output(const uint8_t pwm_pin_number) override;
    bool set_enable_output(const uint8_t enable_pin_number) override;


    uint8_t get_enable_output() override;                              // ← Add this
    bool set_enable_polarity(const ioPolarity new_polarity) override;  // ← Add this  
    ioPolarity get_enable_polarity() override;                         // ← Add this


    void set_frequency(float new_frequency) override;
    float get_frequency() override;

    // trivial getters and setters - inlined
    void set_speed_min(float new_speed_min) override { speed_min = new_speed_min; }
    float get_speed_min() override { return speed_min; }
    void set_speed_max(float new_speed_max) override { speed_max = new_speed_max; }
    float get_speed_max() override { return speed_max; }
    // void set_speed_change_per_tick(float new_speed_change_per_tick) override { speed_change_per_tick = new_speed_change_per_tick; }
    // float get_speed_change_per_tick() override { return speed_change_per_tick; }
    // void set_spinup_delay(float new_spinup_delay) override { spinup_delay = new_spinup_delay; }
    // float get_spinup_delay() override { return spinup_delay; }

    // void set_cw_speed_lo(float new_speed_lo) override { cw.speed_lo = new_speed_lo; }
    // float get_cw_speed_lo() override { return cw.speed_lo; }
    // void set_cw_speed_hi(float new_speed_hi) override { cw.speed_hi = new_speed_hi; }
    // float get_cw_speed_hi() override { return cw.speed_hi; }
    // void set_cw_phase_lo(float new_phase_lo) override { cw.phase_lo = new_phase_lo; }
    // float get_cw_phase_lo() override { return cw.phase_lo; }
    // void set_cw_phase_hi(float new_phase_hi) override { cw.phase_hi = new_phase_hi; }
    // float get_cw_phase_hi() override { return cw.phase_hi; }

    // void set_ccw_speed_lo(float new_speed_lo) override { ccw.speed_lo = new_speed_lo; }
    // float get_ccw_speed_lo() override { return ccw.speed_lo; }
    // void set_ccw_speed_hi(float new_speed_hi) override { ccw.speed_hi = new_speed_hi; }
    // float get_ccw_speed_hi() override { return ccw.speed_hi; }
    // void set_ccw_phase_lo(float new_phase_lo) override { ccw.phase_lo = new_phase_lo; }
    // float get_ccw_phase_lo() override { return ccw.phase_lo; }
    // void set_ccw_phase_hi(float new_phase_hi) override { ccw.phase_hi = new_phase_hi; }
    // float get_ccw_phase_hi() override { return ccw.phase_hi; }

    // void set_phase_off(float new_phase_off) override { phase_off = new_phase_off; }
    // float get_phase_off() override { return phase_off; }

// Stepper functions ----

    void _enableImpl() override;
    void _disableImpl() override;
    void stepStart() override;
    void stepEnd() override;
    void setDirection(uint8_t new_direction) override;
    void setPowerLevels(float active_pl, float idle_pl) override;

// Kinematics functions ----
    uint8_t laser_motor_axis = 4;
    void configure(const float steps_per_unit[MOTORS], const int8_t motor_map[MOTORS]) override;
    void inverse_kinematics(const GCodeState_t &gm, const float target[AXES], const float position[AXES], const float start_velocity, const float end_velocity, const float segment_time, float steps[MOTORS]) override;

// Laser-specific configuration commands
    void set_pulse_duration_us(int16_t new_pulse_duration_us);
    int16_t get_pulse_duration_us();

    float get_min_s();
    void set_min_s(float new_min_s);

    float get_max_s();
    void set_max_s(float new_max_s);

    float get_min_ppm();
    void set_min_ppm(float new_min_ppm);

    float get_max_ppm();
    void set_max_ppm(float new_max_ppm);
};

// IMPLEMENTATION

template <typename KinematicsParent, Motate::pin_number fire_num>
LaserTool<KinematicsParent,fire_num>::LaserTool(const uint8_t enable_pin_number, const uint8_t laser_motor_number)
    : ToolHead{},
      Stepper{},
      KinematicsParent{},
      fire{}, // fire_polarity==IO_ACTIVE_LOW?kStartHigh:kStartLow
      enable_output_num{enable_pin_number},
      laser_motor{laser_motor_number} {}

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::init()
{
    // TODO - ensure outputs are within range
    set_enable_output(enable_output_num);
    fire.clear();
}

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::pause() {
    if (paused) {
        return;
    }

    paused = true;
    fire.clear();
    this->complete_change();
}

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::resume() {
    if (!paused) {
        return;
    }

    paused = false;
    this->complete_change();
}

template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::ready_to_resume() { return paused && safety_manager->ok_to_spindle(); }
// template <typename KinematicsParent, Motate::pin_number fire_num>
// bool LaserTool<KinematicsParent,fire_num>::busy() {
//     // return true when not paused, on, and ramping up to speed
//     if (paused || (direction == SPINDLE_OFF)) {
//         return false;
//     }
//     return true;
// }

template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_speed() { return speed; }

// In set_speed() - only fire immediately for M3, not M4:

// template <typename KinematicsParent, Motate::pin_number fire_num>
// bool LaserTool<KinematicsParent,fire_num>::set_speed(float new_speed) {
//     speed = new_speed;
//     return true;  // Buffer the command
// }


// template <typename KinematicsParent, Motate::pin_number fire_num>
// bool LaserTool<KinematicsParent,fire_num>::set_speed(float new_speed) {
//     speed = new_speed;

//     float override_factor = speed_override_enable ? speed_override_factor : 1.0;
//     float s = std::min(1.0f, std::max(0.0f, (((speed * override_factor) - min_s) / (max_s - min_s))));

//     // ONLY fire immediately for M3 (SPINDLE_CW), NOT M4 (SPINDLE_CCW)
//     if (direction == /*M3 only*/ SPINDLE_CW) {
//         uint32_t top_value = fire.getTopValue();
//         raw_fire_duty_cycle = std::floor(s * (float)top_value);
        
//         // M3 = Static PWM mode - fire immediately
//         current_mode = LASER_MODE_STATIC;
//         fire.writeRaw(raw_fire_duty_cycle);
//     } else if (direction == /*M4 only*/ SPINDLE_CCW) {
//         uint32_t top_value = fire.getTopValue();
//         raw_fire_duty_cycle = std::floor(s * (float)top_value);
        
//         // M4 = Motion-synchronized mode - prepare but don't fire yet
//         current_mode = LASER_MODE_MOTION;
//         // DON'T call fire.writeRaw() here!
//     }

//     return true;
// }


// TODO - make these work
// set the override value for spindle speed
template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::set_override(float override) {
    speed_override_factor = override;
    //set_speed(speed); // use the set_speed() function to update the pin PWM
    return (true);
}
template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_override() { return speed_override_factor; }

// enable or disable the override
template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::set_override_enable(bool override_enable) {
    speed_override_enable = override_enable;
    //set_speed(speed); // use the set_speed() function to update the pin PWM
    return (true);
}
template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::get_override_enable() { return speed_override_enable; }

template <typename KinematicsParent, Motate::pin_number fire_num>
spDirection LaserTool<KinematicsParent,fire_num>::get_direction() { return direction; }

// template <typename KinematicsParent, Motate::pin_number fire_num>
// bool LaserTool<KinematicsParent,fire_num>::set_direction(spDirection new_direction) {
//     direction = new_direction;
//     return true;  // Buffer the command
// }
// template <typename KinematicsParent, Motate::pin_number fire_num>
// bool LaserTool<KinematicsParent,fire_num>::set_direction(spDirection new_direction) {
//     direction = new_direction;
    
//     if (direction == /*M3*/ SPINDLE_CW || direction == /*M4*/ SPINDLE_CCW) {
//         set_speed(speed); // Update fire pin PWM
//     } else if (direction == /*M5*/ SPINDLE_OFF) {
//         // Force fire off immediately and reset mode
//         fire.clear();
//         fire.writeRaw(0);  // ← Add explicit zero PWM
//         current_mode = LASER_MODE_STATIC;
//     }
    
//     this->complete_change(); // This will handle M5 on the fire pin, or not
//     return true; 
// }


template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::stop() {
    paused = false;
    speed = 0;
    direction = SPINDLE_OFF;
    fire.clear();  // ← Add this line

    this->complete_change();
}

// called from a command that was queued when the default set_speed and set_direction returned true
// ALSO called from the loader right before a move
// we are handed the gcode model to use
template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::engage(const GCodeState_t &gm) {
    if ((direction == gm.spindle_direction) && fp_EQ(speed, gm.spindle_speed)) {
        return; // nothing changed
    }

    // Update state from G-code model
    speed = gm.spindle_speed;
    direction = gm.spindle_direction;

    // Execute laser logic based on direction
    if (direction == SPINDLE_CW) {
        // M3 - Static PWM mode
        current_mode = LASER_MODE_STATIC;
        float override_factor = speed_override_enable ? speed_override_factor : 1.0;
        float s = std::min(1.0f, std::max(0.0f, (((speed * override_factor) - min_s) / (max_s - min_s))));
        uint32_t top_value = fire.getTopValue();
        raw_fire_duty_cycle = std::floor(s * (float)top_value);
        fire.writeRaw(raw_fire_duty_cycle);
        
    } else if (direction == SPINDLE_CCW) {
        // M4 - Motion-synchronized mode  
        current_mode = LASER_MODE_MOTION;
        float override_factor = speed_override_enable ? speed_override_factor : 1.0;
        float s = std::min(1.0f, std::max(0.0f, (((speed * override_factor) - min_s) / (max_s - min_s))));
        uint32_t top_value = fire.getTopValue();
        raw_fire_duty_cycle = std::floor(s * (float)top_value);
        // Don't fire yet - wait for motion
        
    } else if (direction == SPINDLE_OFF) {
        // M5 - Turn off
        current_mode = LASER_MODE_STATIC;
        fire.clear();
        fire.writeRaw(0);
    }

    // Update enable pin
    this->complete_change();
}

template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::is_on() { return (direction != SPINDLE_OFF); }

// LaserTool-specific functions
template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::set_pwm_output(const uint8_t) {
    // sometimes I whatever
    return false;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::set_enable_output(const uint8_t enable_pin_number) {
    if (enable_pin_number == 0) {
        enable_output = nullptr;
        return false;
    } else {
        enable_output = d_out[enable_pin_number - 1];
        enable_output->setEnabled(IO_ENABLED);
        enable_output->setPolarity(IO_ACTIVE_HIGH); 

        // set the polarity on the output -- not here
    }
    return true;
}


template <typename KinematicsParent, Motate::pin_number fire_num>
uint8_t LaserTool<KinematicsParent,fire_num>::get_enable_output() {
    return enable_output_num;
}

template <typename KinematicsParent, Motate::pin_number fire_num>
bool LaserTool<KinematicsParent,fire_num>::set_enable_polarity(const ioPolarity new_polarity) {
    if (enable_output != nullptr) {
        enable_output->setPolarity(new_polarity);
        return true;
    }
    return false;
}

template <typename KinematicsParent, Motate::pin_number fire_num>
ioPolarity LaserTool<KinematicsParent,fire_num>::get_enable_polarity() {
    if (enable_output != nullptr) {
        return enable_output->getPolarity();
    }
    return IO_ACTIVE_HIGH;  // default
}


template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_frequency(float new_frequency)
{
    // use to set pulse width
}
template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_frequency()
{
    return 0.0;
}

// Stepper functons

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::_enableImpl() {
    ticks_per_pulse =  next_ticks_per_pulse;
    // fire.writeRaw(raw_fire_duty_cycle);
    enabled = true;
};

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::_disableImpl() {
    fire.writeRaw(0);
    enabled = false;
};

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::stepStart() {
    if (!enabled || current_mode != LASER_MODE_MOTION) return;

    fire.writeRaw(raw_fire_duty_cycle);
    pulse_tick_counter = ticks_per_pulse;
};

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::stepEnd() {
    if (current_mode != LASER_MODE_MOTION || pulse_tick_counter <= 0) {
        return;
    }
    if (--pulse_tick_counter == 0) {
        fire.writeRaw(0);
    }
};

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::setDirection(uint8_t new_direction) {
};

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::setPowerLevels(float active_pl, float idle_pl) {
    ; // ignore this
};


// Private functions

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::complete_change() {
    // if the spindle is not on (or paused), make sure we stop it
    if (paused || direction == SPINDLE_OFF) {
        if (enable_output != nullptr) {
            enable_output->setValue(false);
        }
        fire.clear();  // Always clear fire on M5
        current_mode = LASER_MODE_STATIC;  // Reset mode
        return;
    } else if (direction == SPINDLE_CW) {
        // M3 - Static mode, enable line on
        if (enable_output != nullptr) {
            enable_output->setValue(true);
        }
        current_mode = LASER_MODE_STATIC;
        // Fire PWM is handled by set_speed()
    } else if (direction == SPINDLE_CCW) {
        // M4 - Motion mode, enable line on, but no fire until motion
        if (enable_output != nullptr) {
            enable_output->setValue(true);
        }
        current_mode = LASER_MODE_MOTION;
        // DON'T fire here - wait for motion
    }
}

// Kinematics functions 

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::configure(const float new_steps_per_unit[MOTORS], const int8_t new_motor_map[MOTORS]) {
    int8_t adjusted_motor_map[MOTORS];
    laser_motor_axis = new_motor_map[laser_motor];
    for (uint8_t motor = 0; motor < MOTORS; motor++) {
        if (motor == laser_motor) {
            adjusted_motor_map[motor] = -1; // keep the parent kinematics from computing this motor
            continue;
        }
        adjusted_motor_map[motor] = new_motor_map[motor];
    }
    KinematicsParent::configure(new_steps_per_unit, adjusted_motor_map);
}


template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_pulse_duration_us(int16_t new_pulse_duration_us) {
    pulse_duration_us = new_pulse_duration_us;
}

template <typename KinematicsParent, Motate::pin_number fire_num>
int16_t LaserTool<KinematicsParent,fire_num>::get_pulse_duration_us() {
    return pulse_duration_us;
}

template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_min_s() {
    return min_s;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_min_s(float new_min_s) {
    min_s = new_min_s;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_max_s() {
    return max_s;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_max_s(float new_max_s) {
    max_s = new_max_s;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_min_ppm() {
    return min_ppm;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_min_ppm(float new_min_ppm) {
    min_ppm = new_min_ppm;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
float LaserTool<KinematicsParent,fire_num>::get_max_ppm() {
    return max_ppm;
}
template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::set_max_ppm(float new_max_ppm) {
    max_ppm = new_max_ppm;
}

template <typename KinematicsParent, Motate::pin_number fire_num>
void LaserTool<KinematicsParent,fire_num>::inverse_kinematics(const GCodeState_t &gm, const float target[AXES], const float position[AXES], const float start_velocity, const float end_velocity, const float segment_time, float steps[MOTORS]) {
    
    KinematicsParent::inverse_kinematics(gm, target, position, start_velocity, end_velocity, segment_time, steps);

    float move_length = 0;
    next_ticks_per_pulse = 0;

    // ONLY fire the laser for G1, G2, or G3, when M4 is on (motion-synchronized), and S > 0
    if (!paused && (gm.tool == LASER_TOOL) && 
        ((gm.motion_mode == MOTION_MODE_STRAIGHT_FEED) || (gm.motion_mode == MOTION_MODE_CW_ARC) || (gm.motion_mode == MOTION_MODE_CCW_ARC)) && 
        (gm.spindle_speed > min_s) && (gm.spindle_direction == SPINDLE_CCW)) {  // ← Only for M4!
        
        float s = std::min(1.0f, std::max(0.0f, ((gm.spindle_speed - min_s)/(max_s-min_s)) ));

        float tpp = pulse_duration_us / (1000000 / FREQUENCY_DDA);
        
        // Calculate move_length...
        float x_len = position[AXIS_X] - target[AXIS_X];
        float y_len = position[AXIS_Y] - target[AXIS_Y];
        move_length = sqrt((x_len * x_len) + (y_len * y_len)) * (s * (max_ppm - min_ppm)  + min_ppm);
        
        next_ticks_per_pulse = std::ceil(tpp);
        uint32_t top_value = fire.getTopValue();
        raw_fire_duty_cycle = top_value; // 100%!
    }
    // DON'T change mode here - let M3/M4 commands control the mode

    laser_step_position += move_length;
    steps[laser_motor] = laser_step_position;
}

#endif  // End of include guard: LASER_TOOLHEAD_H_ONCE
