/*
 * coolant.h - coolant driver
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

#ifndef COOLANT_H_ONCE
#define COOLANT_H_ONCE

typedef enum {
    COOLANT_OFF = 0,                // don't change the order. It gets masked.
    COOLANT_ON,
    COOLANT_PAUSE
} cmCoolantEnable;

typedef enum {
    COOLANT_ACTIVE_LOW = 0,
    COOLANT_ACTIVE_HIGH
} cmCoolantPolarity;

typedef enum {                      // used to index the value and flag vectors for coolant execs
    COOLANT_FLOOD = 0,
    COOLANT_MIST,
	 OUT3,
	 OUT4,
	 OUT5,
	 OUT6
} cmCoolantIndex;

typedef enum {                      // used to index the value and flag vectors for coolant execs
	 OUT7 = 0,
	 OUT8,
	 OUT10,
	 OUT11,
	 OUT12
} cmOutputIndex;

typedef enum {                      // used to index the value and flag vectors for coolant execs
	 OUT105 = 0,
	 OUT106,
	 OUT107,
	 OUT108,
	 OUT101
} cmLEDIndex;


/*
 * Coolant control structure
 */

typedef struct cmCoolatSingleton {

    bool pause_on_hold;                 // true to pause coolant on feedhold

	cmCoolantEnable flood_enable;       // on (M62P14), off (M63P14)
    cmCoolantPolarity flood_polarity;   // 0=active low, 1=active high

	cmCoolantEnable mist_enable;        //on (M62P13), off (M63P13)
    cmCoolantPolarity mist_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out3_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out3_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out4_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out4_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out5_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out5_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out6_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out6_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out7_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out7_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out8_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out8_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out10_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out10_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out11_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out11_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out12_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out12_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out101_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out101_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out105_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out105_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out106_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out106_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out107_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out107_polarity;    // 0=active low, 1=active high

	cmCoolantEnable out108_enable;        // on (M62P'pin#'), off (M63P'pin#')
    cmCoolantPolarity out108_polarity;    // 0=active low, 1=active high

} cmCoolantSingleton_t;
extern cmCoolantSingleton_t coolant;


/*
 * Global Scope Functions
 */

void coolant_init();
void coolant_reset();

stat_t cm_mist_coolant_control(uint8_t mist_enable); 
stat_t cm_flood_coolant_control(uint8_t flood_enable);
			
stat_t cm_out3_control(uint8_t out3_enable);
stat_t cm_out4_control(uint8_t out4_enable);
stat_t cm_out5_control(uint8_t out5_enable);
stat_t cm_out6_control(uint8_t out6_enable);
stat_t cm_out7_control(uint8_t out7_enable);
stat_t cm_out8_control(uint8_t out8_enable);
stat_t cm_out10_control(uint8_t out10_enable);
stat_t cm_out11_control(uint8_t out11_enable);
stat_t cm_out12_control(uint8_t out12_enable);

stat_t cm_out101_control(uint8_t out101_enable);
stat_t cm_out105_control(uint8_t out105_enable);
stat_t cm_out106_control(uint8_t out106_enable);
stat_t cm_out107_control(uint8_t out107_enable);
stat_t cm_out108_control(uint8_t out108_enable);

void cm_coolant_off_immediate(void);
void cm_coolant_optional_pause(bool option);
void cm_coolant_resume(void);

/*--- text_mode support functions ---*/

#ifdef __TEXT_MODE

    void cm_print_coph(nvObj_t *nv);     // coolant pause on hold
    void cm_print_comp(nvObj_t *nv);     // coolant polarity mist
    void cm_print_cofp(nvObj_t *nv);     // coolant polarity flood
    void cm_print_com(nvObj_t *nv);     // report mist coolant state
    void cm_print_cof(nvObj_t *nv);     // report flood coolant state

    void cm_print_out3p(nvObj_t *nv); 
    void cm_print_out3(nvObj_t *nv);

    void cm_print_out4p(nvObj_t *nv); 
    void cm_print_out4(nvObj_t *nv);

    void cm_print_out5p(nvObj_t *nv); 
    void cm_print_out5(nvObj_t *nv);

    void cm_print_out6p(nvObj_t *nv); 
    void cm_print_out6(nvObj_t *nv);

    void cm_print_out7p(nvObj_t *nv); 
    void cm_print_out7(nvObj_t *nv);

    void cm_print_out8p(nvObj_t *nv); 
    void cm_print_out8(nvObj_t *nv);

    void cm_print_out10p(nvObj_t *nv); 
    void cm_print_out10(nvObj_t *nv);

    void cm_print_out11p(nvObj_t *nv); 
    void cm_print_out11(nvObj_t *nv);

    void cm_print_out12p(nvObj_t *nv); 
    void cm_print_out12(nvObj_t *nv);

    void cm_print_out101p(nvObj_t *nv); 
    void cm_print_out101(nvObj_t *nv);

    void cm_print_out105p(nvObj_t *nv); 
    void cm_print_out105(nvObj_t *nv);

    void cm_print_out106p(nvObj_t *nv); 
    void cm_print_out106(nvObj_t *nv);

    void cm_print_out107p(nvObj_t *nv); 
    void cm_print_out107(nvObj_t *nv);

    void cm_print_out108p(nvObj_t *nv); 
    void cm_print_out108(nvObj_t *nv);


#else

    #define cm_print_coph tx_print_stub
    #define cm_print_comp tx_print_stub
    #define cm_print_cofp tx_print_stub
    #define cm_print_com tx_print_stub
    #define cm_print_cof tx_print_stub

    #define cm_print_out3p tx_print_stub
    #define cm_print_out3 tx_print_stub

    #define cm_print_out4p tx_print_stub
    #define cm_print_out4 tx_print_stub

    #define cm_print_out5p tx_print_stub
    #define cm_print_out5 tx_print_stub

    #define cm_print_out6p tx_print_stub
    #define cm_print_out6 tx_print_stub

    #define cm_print_out7p tx_print_stub
    #define cm_print_out7 tx_print_stub

    #define cm_print_out8p tx_print_stub
    #define cm_print_out8 tx_print_stub

    #define cm_print_out10p tx_print_stub
    #define cm_print_out10 tx_print_stub

    #define cm_print_out11p tx_print_stub
    #define cm_print_out11 tx_print_stub

    #define cm_print_out12p tx_print_stub
    #define cm_print_out12 tx_print_stub

    #define cm_print_out101p tx_print_stub
    #define cm_print_out101 tx_print_stub

    #define cm_print_out105p tx_print_stub
    #define cm_print_out105 tx_print_stub

    #define cm_print_out106p tx_print_stub
    #define cm_print_out106 tx_print_stub

    #define cm_print_out107p tx_print_stub
    #define cm_print_out107 tx_print_stub

    #define cm_print_out108p tx_print_stub
    #define cm_print_out108 tx_print_stub


#endif // __TEXT_MODE

#endif	// End of include guard: COOLANT_H_ONCE
