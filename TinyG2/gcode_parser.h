/*
 * gcode_parser.h - rs274/ngc Gcode parser
 * This file is part of the TinyG project
 *
 * Copyright (c) 2010 - 2015 Alden S. Hart, Jr.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef GCODE_PARSER_H_ONCE
#define GCODE_PARSER_H_ONCE

/*
 * Global Scope Functions
 */
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

    #define _set_out101_enable_bit_hi() out101_enable_pin.set()
    #define _set_out101_enable_bit_lo() out101_enable_pin.clear()

    #define _set_out105_enable_bit_hi() out105_enable_pin.set()
    #define _set_out105_enable_bit_lo() out105_enable_pin.clear()

    #define _set_out106_enable_bit_hi() out106_enable_pin.set()
    #define _set_out106_enable_bit_lo() out106_enable_pin.clear()

    #define _set_out107_enable_bit_hi() out107_enable_pin.set()
    #define _set_out107_enable_bit_lo() out107_enable_pin.clear()

    #define _set_out108_enable_bit_hi() out108_enable_pin.set()
    #define _set_out108_enable_bit_lo() out108_enable_pin.clear()

stat_t gcode_parser(char *block);
stat_t gc_get_gc(nvObj_t *nv);
stat_t gc_run_gc(nvObj_t *nv);

#endif // End of include guard: GCODE_PARSER_H_ONCE
