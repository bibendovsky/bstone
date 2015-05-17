BStone
======

A source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike.


Contents
========

1. Disclaimer
2. Profile
3. Compiling
4. Command-line options
5. Cheat key
6. Debug keys
7. Third party use
8. Credits
9. Links


1 - Disclaimer
==============

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013-2015 Boris I. Bendovsky (<bibendovsky@hotmail.com>)

This program is free software; you can redistribute it and/or  
modify it under the terms of the GNU General Public License  
as published by the Free Software Foundation; either version 2  
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the  
GNU General Public License for more details.

You should have received a copy of the GNU General Public License  
along with this program; if not, write to the Free Software  
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

---

For a copy of the GNU General Public License see file LICENSE.  
For an original source code license see file "Blake Stone source code license.doc".


2 - Profile
===========

The port stores configuration file, saved game files, etc. in user's profile.  
The path to those files depends on platform.

On Windows, the path might look like:  
"C:\\Users\\john\\AppData\\Roaming\\bibendovsky\\bstone\\"

On Linux, the path might look like:  
"/home/john/.local/share/bstone/"

On Mac OS X, the path might look like:  
"/Users/john/Library/Application Support/bstone/"

Configuration file name: bstone_config  
Log file name: bstone_log.txt  
High scores file name: bstone_game_high_scores  
Saved game file name: bstone_game_saved_game

Where "game" is:  
* aog_sw - Aliens of Gold (shareware)  
* aog_full - Aliens of Gold (full)  
* ps - Planet Strike


3 - Compiling
=============

Minimum requirements:
* C++11 compatible compiler.  
  Tested with Visual C++ 12 and GCC 4.9.2.

* CMake 2.8  
  (<http://cmake.org/>).

* pkg-config (only for non Visual C++ compiler)  
  (<http://pkg-config.freedesktop.org/>)

* SDL v2.0.1 (non-Windows) / SDL v2.0.3 (Windows)  
  (<http://www.libsdl.org/>)

CMake variables:
* CMAKE_BUILD_TYPE  
  Selects wich build(s) to compile.  
  Use semicolon to separate entries. 
  Usually it's Debug or Release.  
  For other values see CMake documentation.

* BSTONE_PANDORA  
  If enabled prepares build for Open Pandora.

* BSTONE_USE_PCH  
  If enabled utilizes precompiled headers to speed up compilation.  
  Note: Visual C++ only

* BSTONE_USE_STATIC_LINKING  
  If enabled links modules statically to avoid dependancy on  
  system and custom libraries at run-time.

* SDL2_INCLUDE_DIRS  
  Defines directory with SDL2 headers.  
  Note: Visual C++ only

* SDL2_LIBRARIES  
  Defines list of SDL2 libraries.  
  Use semicolon to separate entries.  
  Note: Visual C++ only

Notes:
* Use ON value to enable option and value OFF to disable option.


4 - Command-line options
========================

* --version  
  Outputs the port's version to standard output and  
  into message box.

* --aog_sw  
  Switches the port to Blake Stone: Aliens of Gold (shareware) mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG (full) -> AoG (sw) -> PS

* --aog_full  
  Switches the port to Blake Stone: Aliens of Gold (full) mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG (full) -> AoG (sw) -> PS

* --ps  
  Switches the port to Blake Stone: Planet Strike mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG (full) -> AoG (sw) -> PS

* --no_screens  
  Skips startup screens and promo pages (AoG sw only).

* --cheats  
  Enables so called "debug mode" without much fuss.

* --data_dir path_to_data  
  Specifies location to the game's data files.  
  Default: current working directory.

* --vid_renderer [soft|ogl]  
  Forces to use a specified renderer.  
  "soft" selects a software renderer.  
  "ogl" selects an OpenGL 2.x compatible renderer.  
  Default order without this option: ogl, soft.

* --vid_windowed  
  Runs the game in windowed mode.  
  Default video mode: 640x480

* --vid_mode width height  
  Selects the specified resolution for windowed mode.  
  Without this option the game will use desktop's resolution.  
  Minimum width: 640  
  Minimum height: 480

* --vid_scale factor  
  Refinement factor. The higher a value the greater internal resolution  
  mode will be used to render a scene. The dimensions of the resolution mode  
  are proportional to the original one (320x200) by 'factor' value.  
  This option can greatly affect the performance of a renderer (especially a  
  software one).  
  Minimum factor: 1 (identical to the original game)  
  Default factor: depends on the game's resolution mode.

* --vid_window_x offset  
  Sets a horizontal offset from the left side of the desktop screen.  
  Applicable for windowed mode only.

* --vid_window_y offset  
  Sets a vertical offset from the top side of the desktop screen.  
  Applicable for windowed mode only.

* --vid_no_fix_par  
  Disables correction of pixel aspect ratio.  
  By default the port stretches a height of rendererd screen by 20% to  
  comply with original VGA's pixel aspect ratio.

* --snd_rate sampling_rate  
  Specifies sampling rate of mixer in hertz.  
  Default: 22050 Hz  
  Minimum: 11025 Hz

* --snd_mix_size duration  
  Specifies mix data size in milliseconds.  
  Default: 40 ms  
  Minimum: 20 ms


5 - Cheat key
=============

[J] [A] [M] [Enter]  
Press specified keys sequentially.  
Shows message "NOW you're jammin'!!", and gives to you all keys,  
all weapons and restores health to 100% but zeroes score points.  
Not available in shareware version.


6 - Debug keys
==============

Add option "--cheats" to enable these keys.

* [Backspace]+[A]  
  Toggles visibility of actors on auto-map.

* [Backspace]+[C]  
  Shows counts of total static objects, in use static objects,  
  doors, total actors, active actors.

* [Backspace]+[D]  
  Toggles player's invisibility.

* [Backspace]+[E]  
  Win mission instantly.

* [Backspace]+[F]  
  Shows player's coordinates and direction.

* [Backspace]+[G]  
  Toggles god mode.

* [Backspace]+[H]  
  Hurt yourself by 1%.

* [Backspace]+[I]  
  Each usege adds 99% health, 5 tokens, 50 ammo, one new weapon.

* [Backspace]+[K]  
  Shows total counts of enemies, points and informants on the map.

* [Backspace]+[M]  
  Memory information.

* [Backspace]+[O]  
  Shows push walls on auto-map.

* [Backspace]+[Q]  
  Instant quit.

* [Backspace]+[R]  
  Shows full map.

* [Backspace]+[S]  
  Slow motion.

* [Backspace]+[U]  
  Unlocks all maps.

* [Backspace]+[W]  
  Warps to specified map.

* [Backspace]+[Shift/Caps Lock]+[W]  
  Warps to specified map but loads it from scratch  
  rather from a saved game file/memory.

* [Backspace]+[Home]  
  Selects previous ceiling texture.

* [Backspace]+[Page Up]  
  Selects next ceiling texture.

* [Backspace]+[End]  
  Selects previous flooring texture.

* [Backspace]+[Page Down]  
  Selects next flooring texture.

* [Backspace]+[-]  
  Decreases shading depth.

* [Backspace]+[=]  
  Increases shading depth.

* [Backspace]+['[']  
  Decreases shading drop off.

* [Backspace]+[']']  
  Increases shading drop off.


7 - Third party use
===================

* Simple DirectMedia Library (v2)  
  <http://libsdl.org/>  
  See file COPYING-SDL2.txt for a license information.

* DOSBox  
  <http://www.dosbox.com/>  
  See file dosbox/COPYING for a license information.  
  Note: The source port uses only an OPL emulation code.


8 - Credits
===========

* id Software  
  Developing Wolfenstein 3D engine.  
  Web-site: <http://www.idsoftware.com/>

* JAM Productions  
  Developing the game.

* Apogee Entertainment, LLC  
  Publishing the game and releasing a source code.  
  Web-site: <http://www.apogeesoftware.com/>

* Boris I. Bendovsky  
  Author of the source code.  
  Email: <bibendovsky@hotmail.com>

* Scott Smith  
  Adaptation to Pandora console, various fixes.


8 - Links
=========

* The port's source code and binary releases:  
  <https://github.com/bibendovsky/bstone/>

* Apogee's article about releasing of an original source code:  
  <http://www.apogeesoftware.com/uncategorized/apogee-releases-blake-stone-source-code>
