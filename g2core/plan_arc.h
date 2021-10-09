/*
 * plan_arc.h - arc planning and motion execution
 * This file is part of the g2core project
 *
 * Copyright (c) 2010 - 2018 Alden S. Hart, Jr.
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

#ifndef PLAN_ARC_H_ONCE
#define PLAN_ARC_H_ONCE

#define MIN_ARC_RADIUS ((float)0.1)             // min radius that can be executed
#define MIN_ARC_SEGMENT_LENGTH ((float)0.05)    // Arc segment size (mm).(0.03)
#define MIN_ARC_SEGMENT_USEC ((float)10000)     // minimum arc segment time

// Arc radius tests. See http://linuxcnc.org/docs/html/gcode/gcode.html#sec:G2-G3-Arc
//#define ARC_RADIUS_ERROR_MAX ((float)0.5)     // max allowable mm between start and end radius
#define ARC_RADIUS_ERROR_MAX ((float)1.0)       // max allowable mm between start and end radius
#define ARC_RADIUS_ERROR_MIN ((float)0.005)     // min mm where 1% rule applies
#define ARC_RADIUS_TOLERANCE ((float)0.001)     // 0.1% radius variance test

#define CHORDAL_TOLERANCE_MIN (0.001)           // values below this are not accepted

/* arc function prototypes */

void   cm_arc_init(cmMachine_t *_cm);
void   cm_abort_arc(cmMachine_t *_cm);
stat_t cm_arc_callback(cmMachine_t *_cm);

#endif  // End of include guard: PLAN_ARC_H_ONCE
