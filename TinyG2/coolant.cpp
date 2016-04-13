/*
 * coolant.cpp - canonical machine coolant driver
 * This file is part of the TinyG project
 *
 * Copyright (c) 2015 Alden S. Hart, Jr.
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

#include "tinyg2.h"             // #1 dependency order
#include "config.h"             // #2
#include "canonical_machine.h"  // #3
#include "text_parser.h"        // #4

#include "coolant.h"
#include "planner.h"
#include "hardware.h"
#include "util.h"

/**** Allocate structures ****/

cmCoolantSingleton_t coolant;

/**** Static functions ****/

static void _exec_coolant_control(float *value, bool *flags);

static void _exec_output_control(float *value, bool *flags);

/*
 * coolant_init()
 * coolant_reset()
 */
void coolant_init()
{
    coolant.mist_enable = COOLANT_OFF;
    coolant.flood_enable = COOLANT_OFF;
    coolant.out3_enable = COOLANT_OFF;
    coolant.out4_enable = COOLANT_OFF;
    coolant.out5_enable = COOLANT_OFF;
    coolant.out6_enable = COOLANT_OFF;
    coolant.out7_enable = COOLANT_OFF;
    coolant.out8_enable = COOLANT_OFF;
    coolant.out10_enable = COOLANT_OFF;
    coolant.out11_enable = COOLANT_OFF;
    coolant.out12_enable = COOLANT_OFF;
}

void coolant_reset()
{
    coolant_init();
    cm_coolant_off_immediate();
}

/*
 * cm_coolant_off_immediate() - turn off all coolant
 * cm_coolant_optional_pause() - pause coolants if option is true
 * cm_coolant_resume() - restart paused coolants
 */

void cm_coolant_off_immediate()
{
    float value[] = { 0,0,0,0,0,0 };
    bool flags[] = { 1,1,0,0,0,0 };
    _exec_coolant_control(value, flags);
}

void cm_coolant_optional_pause(bool option)
{
    if (option) {
        float value[] = { 0,0,0,0,0,0 };
        bool flags[] = { 0,0,0,0,0,0 };

        if (coolant.flood_enable == COOLANT_ON) {
            coolant.flood_enable = COOLANT_PAUSE;   // mark as paused
            flags[COOLANT_FLOOD] = 1;               // set flag so it will turn off
        }
        if (coolant.mist_enable == COOLANT_ON) {
            coolant.mist_enable = COOLANT_PAUSE;     // mark as paused
            flags[COOLANT_MIST] = 1;
        }
        _exec_coolant_control(value, flags);        // execute (w/o changing local state)
    }
}

void cm_coolant_resume()
{
//    float value[] = { 1,1,0,0,0,0 };  // ++++ Will this work? No need to set 'value' below
    float value[] = { 0,0,0,0,0,0 };
    bool flags[] = { 0,0,0,0,0,0 };

    if (coolant.flood_enable == COOLANT_PAUSE) {
        coolant.flood_enable = COOLANT_ON;
        value[COOLANT_FLOOD] = 1;
        flags[COOLANT_FLOOD] = true;
    }
    if (coolant.mist_enable == COOLANT_PAUSE) {
        coolant.mist_enable = COOLANT_ON;
        value[COOLANT_MIST] = 1;
        flags[COOLANT_MIST] = true;
    }
    _exec_coolant_control(value, flags);
}

/*
 * cm_mist_coolant_control() - access points from Gcode parser
 * cm_flood_coolant_control() - access points from Gcode parser
 * _exec_coolant_control() - combined flood and mist coolant control
 *
 *  - value[0] is flood state
 *  - value[1] is mist state
 *  - uses flags to determine which to run
 */

stat_t cm_flood_coolant_control(uint8_t flood_enable)
{
    float value[] = { (float)flood_enable, 0,0,0,0,0,0 };
    bool flags[] = { 1,0,0,0,0,0 };
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}

stat_t cm_mist_coolant_control(uint8_t mist_enable)
{
    float value[] = { 0, (float)mist_enable, 0,0,0,0 };
    bool flags[] = { 0,1,0,0,0,0 };
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}


stat_t cm_out3_control(uint8_t out3_enable)
{
    float value[] = { 0,0, (float)out3_enable, 0,0,0 };
    bool flags[] = { 0,0,1,0,0,0 };
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out4_control(uint8_t out4_enable)
{
    float value[] = { 0,0,0,(float)out4_enable, 0,0 };
    bool flags[] = { 0,0,0,1,0,0 };
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out5_control(uint8_t out5_enable)
{
    float value[] = { 0,0,0,0,(float)out5_enable,0 };
    bool flags[] = { 0,0,0,0,1,0};
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out6_control(uint8_t out6_enable)
{
    float value[] = { 0,0,0,0,0,(float)out6_enable };
    bool flags[] = { 0,0,0,0,0,1};
    mp_queue_command(_exec_coolant_control, value, flags);
    return (STAT_OK);
}

//

stat_t cm_out67_control(uint8_t out7_enable)
{
    float value[] = { (float)out7_enable, 0,0,0,0,0,0 };
    bool flags[] = { 1,0,0,0,0,0 };
    mp_queue_command(_exec_output_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out8_control(uint8_t out8_enable)
{
    float value[] = { 0,(float)out8_enable, 0,0,0,0 };
    bool flags[] = { 0,1,0,0,0,0 };
    mp_queue_command(_exec_output_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out10_control(uint8_t out10_enable)
{
    float value[] = { 0,0,(float)out10_enable, 0,0,0 };
    bool flags[] = { 0,0,1,0,0,0 };
    mp_queue_command(_exec_output_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out11_control(uint8_t out11_enable)
{
    float value[] = { 0,0,0,(float)out11_enable, 0,0 };
    bool flags[] = { 0,0,0,1,0,0 };
    mp_queue_command(_exec_output_control, value, flags);
    return (STAT_OK);
}

stat_t cm_out12_control(uint8_t out12_enable)
{
    float value[] = { 0,0,0,0,(float)out12_enable,0 };
    bool flags[] = { 0,0,0,0,1,0 };
    mp_queue_command(_exec_output_control, value, flags);
    return (STAT_OK);
}

#ifdef __ARM
    // NOTE: flood and mist coolants are mapped to the same pin - see hardware.h
    #define _set_flood_enable_bit_hi() flood_enable_pin.set()
    #define _set_flood_enable_bit_lo() flood_enable_pin.clear()
    #define _set_mist_enable_bit_hi() mist_enable_pin.set()
    #define _set_mist_enable_bit_lo() mist_enable_pin.clear()
	 //outputs
    #define _set_out3_enable_bit_hi() out3_enable_pin.set()
    #define _set_out3_enable_bit_lo() out3_enable_pin.clear()

    #define _set_out4_enable_bit_hi() out4_enable_pin.set()
    #define _set_out4_enable_bit_lo() out4_enable_pin.clear()

    #define _set_out5_enable_bit_hi() out5_enable_pin.set()
    #define _set_out5_enable_bit_lo() out5_enable_pin.clear()

    #define _set_out6_enable_bit_hi() out6_enable_pin.set()
    #define _set_out6_enable_bit_lo() out6_enable_pin.clear()

    #define _set_out7_enable_bit_hi() out7_enable_pin.set()
    #define _set_out7_enable_bit_lo() out7_enable_pin.clear()

    #define _set_out8_enable_bit_hi() out8_enable_pin.set()
    #define _set_out8_enable_bit_lo() out8_enable_pin.clear()

    #define _set_out10_enable_bit_hi() out10_enable_pin.set()
    #define _set_out10_enable_bit_lo() out10_enable_pin.clear()

    #define _set_out11_enable_bit_hi() out11_enable_pin.set()
    #define _set_out11_enable_bit_lo() out11_enable_pin.clear()

    #define _set_out12_enable_bit_hi() out12_enable_pin.set()
    #define _set_out12_enable_bit_lo() out12_enable_pin.clear()

#endif
#ifdef __AVR
    #define _set_flood_enable_bit_hi() gpio_set_bit_on(COOLANT_BIT)
    #define _set_flood_enable_bit_lo() gpio_set_bit_off(COOLANT_BIT)
    #define _set_mist_enable_bit_hi() gpio_set_bit_on(COOLANT_BIT)
    #define _set_mist_enable_bit_lo() gpio_set_bit_off(COOLANT_BIT)

    #define _set_out3_enable_bit_hi() gpio_set_bit_on(COOLANT_BIT)
    #define _set_out3_enable_bit_lo() gpio_set_bit_off(COOLANT_BIT)
#endif


static void _exec_coolant_control(float *value, bool *flags)
{
    if (flags[COOLANT_FLOOD]) {
        coolant.flood_enable = (cmCoolantEnable)value[COOLANT_FLOOD];
        if (!((coolant.flood_enable & 0x01) ^ coolant.flood_polarity)) {    // inverted XOR
            _set_flood_enable_bit_hi();
        } else {
            _set_flood_enable_bit_lo();
        }
    }
    if (flags[COOLANT_MIST]) {
        coolant.mist_enable = (cmCoolantEnable)value[COOLANT_MIST];
        if (!((coolant.mist_enable & 0x01) ^ coolant.mist_polarity)) {
            _set_mist_enable_bit_hi();
        } else {
            _set_mist_enable_bit_lo();
        }
    }

    if (flags[OUT3]) {
        coolant.out3_enable = (cmCoolantEnable)value[OUT3];
        if (!((coolant.out3_enable & 0x01) ^ coolant.out3_polarity)) {
            _set_out3_enable_bit_hi();
        } else {
            _set_out3_enable_bit_lo();
        }
    }

    if (flags[OUT4]) {
        coolant.out4_enable = (cmCoolantEnable)value[OUT4];
        if (!((coolant.out4_enable & 0x01) ^ coolant.out4_polarity)) {
            _set_out4_enable_bit_hi();
        } else {
            _set_out4_enable_bit_lo();
        }
    }


    if (flags[OUT5]) {
        coolant.out5_enable = (cmCoolantEnable)value[OUT5];
        if (!((coolant.out5_enable & 0x01) ^ coolant.out5_polarity)) {
            _set_out5_enable_bit_hi();
        } else {
            _set_out5_enable_bit_lo();
        }
    }
    if (flags[OUT6]) {
        coolant.out6_enable = (cmCoolantEnable)value[OUT6];
        if (!((coolant.out6_enable & 0x01) ^ coolant.out6_polarity)) {
            _set_out6_enable_bit_hi();
        } else {
            _set_out6_enable_bit_lo();
        }
    }


}


static void _exec_output_control(float *value, bool *flags){

    if (flags[OUT7]) {
        coolant.out7_enable = (cmCoolantEnable)value[OUT7];
        if (!((coolant.out7_enable & 0x01) ^ coolant.out7_polarity)) {
            _set_out7_enable_bit_hi();
        } else {
            _set_out7_enable_bit_lo();
        }
    }

    if (flags[OUT8]) {
        coolant.out8_enable = (cmCoolantEnable)value[OUT8];
        if (!((coolant.out8_enable & 0x01) ^ coolant.out8_polarity)) {
            _set_out8_enable_bit_hi();
        } else {
            _set_out8_enable_bit_lo();
        }
    }
    if (flags[OUT10]) {
        coolant.out10_enable = (cmCoolantEnable)value[OUT10];
        if (!((coolant.out10_enable & 0x01) ^ coolant.out10_polarity)) {
            _set_out10_enable_bit_hi();
        } else {
            _set_out10_enable_bit_lo();
        }
    }
    if (flags[OUT11]) {
        coolant.out11_enable = (cmCoolantEnable)value[OUT11];
        if (!((coolant.out11_enable & 0x01) ^ coolant.out11_polarity)) {
            _set_out11_enable_bit_hi();
        } else {
            _set_out11_enable_bit_lo();
        }
    }
    if (flags[OUT12]) {
        coolant.out12_enable = (cmCoolantEnable)value[OUT12];
        if (!((coolant.out12_enable & 0x01) ^ coolant.out12_polarity)) {
            _set_out12_enable_bit_hi();
        } else {
            _set_out12_enable_bit_lo();
        }
    }

}
/***********************************************************************************
 * TEXT MODE SUPPORT
 * Functions to print variables from the cfgArray table
 ***********************************************************************************/

#ifdef __TEXT_MODE

const char fmt_coph[] PROGMEM = "[coph] coolant pause on hold%7d [0=no,1=pause_on_hold]\n";
const char fmt_comp[] PROGMEM = "[comp] coolant mist polarity%7d [0=low is ON,1=high is ON]\n";
const char fmt_cofp[] PROGMEM = "[cofp] coolant flood polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_com[] PROGMEM = "Mist coolant:%6d [0=OFF,1=ON]\n";
const char fmt_cof[] PROGMEM = "Flood coolant:%5d [0=OFF,1=ON]\n";

const char fmt_out3p[] PROGMEM = "[out3p] OUTPUT 3 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out3[] PROGMEM = "OUTPUT 3:%5d [0=OFF,1=ON]\n";

const char fmt_out4p[] PROGMEM = "[out4p] OUTPUT 4 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out4[] PROGMEM = "OUTPUT 4:%5d [0=OFF,1=ON]\n";

const char fmt_out5p[] PROGMEM = "[out5p] OUTPUT 5 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out5[] PROGMEM = "OUTPUT 5:%5d [0=OFF,1=ON]\n";

const char fmt_out6p[] PROGMEM = "[out6p] OUTPUT 6 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out6[] PROGMEM = "OUTPUT 6:%5d [0=OFF,1=ON]\n";

const char fmt_out7p[] PROGMEM = "[out7p] OUTPUT 7 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out7[] PROGMEM = "OUTPUT 7:%5d [0=OFF,1=ON]\n";

const char fmt_out8p[] PROGMEM = "[out8p] OUTPUT 8 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out8[] PROGMEM = "OUTPUT 8:%5d [0=OFF,1=ON]\n";

const char fmt_out10p[] PROGMEM = "[out10p] OUTPUT 10 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out10[] PROGMEM = "OUTPUT 10:%5d [0=OFF,1=ON]\n";

const char fmt_out11p[] PROGMEM = "[out11p] OUTPUT 11 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out11[] PROGMEM = "OUTPUT 11:%5d [0=OFF,1=ON]\n";

const char fmt_out12p[] PROGMEM = "[out12p] OUTPUT 12 polarity%6d [0=low is ON,1=high is ON]\n";
const char fmt_out12[] PROGMEM = "OUTPUT 12:%5d [0=OFF,1=ON]\n";

void cm_print_coph(nvObj_t *nv) { text_print(nv, fmt_coph);}  // TYPE_INT
void cm_print_comp(nvObj_t *nv) { text_print(nv, fmt_comp);}  // TYPE_INT
void cm_print_cofp(nvObj_t *nv) { text_print(nv, fmt_cofp);}  // TYPE_INT
void cm_print_com(nvObj_t *nv) { text_print(nv, fmt_com);}    // TYPE_INT
void cm_print_cof(nvObj_t *nv) { text_print(nv, fmt_cof);}    // TYPE_INT

void cm_print_out3p(nvObj_t *nv) { text_print(nv, fmt_out3p);}
void cm_print_out3(nvObj_t *nv) { text_print(nv, fmt_out3);}

void cm_print_out4p(nvObj_t *nv) { text_print(nv, fmt_out4p);}
void cm_print_out4(nvObj_t *nv) { text_print(nv, fmt_out4);}

void cm_print_out5p(nvObj_t *nv) { text_print(nv, fmt_out5p);}
void cm_print_out5(nvObj_t *nv) { text_print(nv, fmt_out5);}

void cm_print_out6p(nvObj_t *nv) { text_print(nv, fmt_out6p);}
void cm_print_out6(nvObj_t *nv) { text_print(nv, fmt_out6);}

void cm_print_out7p(nvObj_t *nv) { text_print(nv, fmt_out7p);}
void cm_print_out7(nvObj_t *nv) { text_print(nv, fmt_out7);}

void cm_print_out8p(nvObj_t *nv) { text_print(nv, fmt_out8p);}
void cm_print_out8(nvObj_t *nv) { text_print(nv, fmt_out8);}

void cm_print_out10p(nvObj_t *nv) { text_print(nv, fmt_out10p);}
void cm_print_out10(nvObj_t *nv) { text_print(nv, fmt_out10);}

void cm_print_out11p(nvObj_t *nv) { text_print(nv, fmt_out11p);}
void cm_print_out11(nvObj_t *nv) { text_print(nv, fmt_out11);}

void cm_print_out12p(nvObj_t *nv) { text_print(nv, fmt_out12p);}
void cm_print_out12(nvObj_t *nv) { text_print(nv, fmt_out12);}

#endif // __TEXT_MODE
