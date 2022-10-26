<img src="https://raw.githubusercontent.com/wiki/synthetos/g2/images/g2core.png" width="300" height="129" alt="g2core">

[![Build Status](https://travis-ci.org/synthetos/g2.svg?branch=edge)](https://travis-ci.org/synthetos/g2) [![Issues in Ready](https://badge.waffle.io/synthetos/g2.svg?label=ready&title=Ready)](http://waffle.io/synthetos/g2) [![Issues in Progress](https://badge.waffle.io/synthetos/g2.svg?label=in%20progress&title=In%20Progress)](http://waffle.io/synthetos/g2)

# What it is

g2core is a 9 axes (XYZABC+UVW) motion control system designed for high-performance on small to mid-sized machines.

* CNC
* 3D printing
* Laser cutting
* Robotics

Our default target is the [Arduino Due](https://store.arduino.cc/arduino-due), though it can also be used with other boards.

Some features:

* 9 axis motion (XYZABC+UVW).
  * **Note** - UVW is only in the `edge` branch for now.
* Jerk controlled motion for acceleration planning (3rd order motion planning)
* Status displays ('?' character)
* XON/XOFF and RTS/CTS protocol over USB serial
* RESTful interface using JSON

# Mailing List

For both user and developer discussions of g2core, we recently created a mailing list:

* https://lists.links.org/mailman/listinfo/g2core

Please feel welcome to join in. :smile:

# g2core - Main Branch

For now, we only recommend using the [main branch](https://github.com/FabMo/FabMo-G2-Core/tree/main).

## Firmware Build 101 `{fb:101.xx}`
### Feature Enhancements
New features added. See linked issues and pull requests for details
- Added [UVW axes](https://github.com/synthetos/g2/wiki/9-Axis-UVW-Operation) for 9 axis control. [See also: Issue 304](https://github.com/synthetos/g2/issues/304)
- Added [Enhanced Feedhold Functions](https://github.com/synthetos/g2/wiki/Feedhold,-Resume,-and-Other-Simple-Commands)
- Added explicit [Job Kill  ^d](https://github.com/synthetos/g2/wiki/Feedhold,-Resume,-and-Other-Simple-Commands#job-kill) - has the effect of an M30 (program end)
- Documented [Communications Startup Tests](https://github.com/synthetos/g2/wiki/g2core-Communications#enqack---checking-for-clean-startup)


### Internal Changes and Bug Fixes
Many things have changed in the internals for this very large pull request. The following list highlights some of these changes but is not meant to be comprehensive.
- Added explicit typing and type testing to JSON variables.
- As part of the above, 32bit integers are not float casts, and therefore retain full accuracy. Line numbers may now reliably go to 2,000,000,000
- Movement towards getters and setters as initial stage of refactoring the Big Table :)
- Bugfix: Fixed root finding problem in feedhold exit velocity calculation
- Bugfix: fixed bug in B and C axis assignment in coordinate rotation code
- PR #334 A, B, C axes radius defaults to use motors 4, 5, & 6
- PR #336, Issue #336 partial solution to coolant initialization
- PR #299, Issue #298 fix for reading nested JSON value errors

## Feature Enhancements

### Firmware Build 101 `{fb:101.xx}`

The fb:101 release is a mostly internal change from the fb:100 branches. Here are the highlights, more detailed on each item are further below:
- Updated motion execution at the segment (smallest) level to be linear velocity instead of constant velocity, resulting in notably smoother motion and more faithful execution of the jerk limitations. (Incidentally, the sound of the motors is also slightly quieter and more "natural.")
- Updated JT (Junction integration Time, a.k.a. "cornering") handling to be more optimized, and to treat the last move as a corner to a move with no active axes. This allows a non-zero stopping velocity based on the allowed jerk and active JT value.
- Probing enhancements.
- Added support for gQuintic (rev B) and fixed issues with gQuadratic board support. (This mostly happened in Motate.)
- Temperature control enhancements
  - Temperature inputs are configured differently at compile time. (Ongoing.)
  - PID control has been adjusted to PID+FF (Proportional, Integral, and Derivative, with Feed Forward). In this case, the feed forward is a multiplier of the difference between the current temperature and the ambient temperature. Since there is no temperature sensor for ambient temperature at the moment, it uses an idealized room temperature of 21ºC.
- More complete support for TMC2130 by adding more JSON controls for live feedback and configuration.
- Initial support for Core XY kinematics.
- Boards are in more control of the planner settings.
- Experimental setting to have traverse (G0) use the 'high jerk' axis settings.
- Outputs are now configured at board initialization (and later) to honor the settings more faithfully. This includes setting the pin high or low as soon as possible.

### Firmware Build 100 `{fb:100.xx}`

The fb:100 release is a major change from the fb:089 and earlier branches. It represents about a year of development and has many major feature enhancements summarized below. These are described in more detail in the rest of this readme and the linked wiki pages.
- New Gcode and CNC features
- 3d printing support, including [Marlin Compatibility](https://github.com/synthetos/g2/wiki/Marlin-Compatibility)
- GPIO system enhancements
- Planner enhancements and other operating improvements for high-speed operation
- Initial support for new processors, including the ARM M7

### Project Changes

The project is now called g2core (even if the repo remains g2). As of this release the g2core code base is split from the TinyG code base. TinyG will continue to be supported for the Xmega 8-bit platform, and new features will be added, specifically as related to continued support for CNC milling applications. The g2core project will focus on various ARM platforms, as it currently does, and add functions that are not possible in the 8-bit platform.

In this release the Motate hardware abstraction layer has been split into a separate project and is included in g2core as a git submodule. This release also provides better support for cross platform / cross target compilation. A summary of project changes is provided below, with details in this readme and linked wiki pages.
- Motate submodule
- Cross platform / cross target support
- Multiple processor support - ARM M3, M4, M7 cores
- Device tree / multiple motor types
- Simplified host-to-board communication protocol (line mode)
- NodeJS host module for host-to-board communications

### More To Come
The fb:100 release is the base for  number of other enhancements in the works and planned, including:
- Further enhancements to GPIO system
- Additional JSON processing and UI support
- Enhancements to 3d printer support, including a simplified g2 printer dialect
