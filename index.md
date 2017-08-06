![BStone: Aliens of Gold screenshot (640x480)](https://raw.githubusercontent.com/bibendovsky/bstone/gh-pages/images/bstone_aog_sample_640x480_01.png)  


![BStone: Planet Strike screenshot (640x480)](https://raw.githubusercontent.com/bibendovsky/bstone/gh-pages/images/bstone_ps_sample_640x480_01.png)


Overview
===

BStone is a source port of Blake Stone game series: [Aliens of Gold](http://wikipedia.org/wiki/Blake_Stone:_Aliens_of_Gold) and [Planet Strike](http://wikipedia.org/wiki/Blake_Stone:_Planet_Strike).

Features:
* High resolution rendering of world (extended vanilla engine)
* Modern and vanilla controls
* Allows to customize control bindings
* Separate volume control of sound effects and music

Supported games:
* Aliens of Gold v1.0 full
* Aliens of Gold v2.0 full
* Aliens of Gold v2.1 full
* Aliens of Gold v3.0 full
* Aliens of Gold v3.0 shareware
* Planet Strike v1.0
* Planet Strike v1.1


Profile
===

The port stores configuration file, saved game files, etc. in user's profile. The path to those files depends on platform. To override the path use --profile_dir option.

On Windows, the path might look like:  
"C:\\Users\\john\\AppData\\Roaming\\bibendovsky\\bstone\\"

On Linux, the path might look like:  
"/home/john/.local/share/bibendovsky/bstone/"

On Mac OS X, the path might look like:  
"/Users/john/Library/Application Support/bibendovsky/bstone/"

Configuration file name: bstone_config  
Log file name: bstone_log.txt  
High scores file name: bstone_game_high_scores  
Saved game file name: bstone_game_saved_game

Where "game" is:
* aog_sw - Aliens of Gold (shareware)
* aog_full - Aliens of Gold (full)
* ps - Planet Strike


Command-line options
===

* --version  
Outputs the port's version to standard output and into message box.

* --aog_sw  
Switches the port to Blake Stone: Aliens of Gold (shareware, v3.0) mode.  
If appropriate data files will not be found the port will fail.  
Default switch strategy: AoG (full) -> AoG (SW) -> PS

* --aog_10  
Switches the port to Blake Stone: Aliens of Gold (full, v1.0) mode.  If appropriate data files will not be found the port will fail.  
Default switch strategy: AoG (full) -> AoG (SW) -> PS

* --aog_2x  
Switches the port to Blake Stone: Aliens of Gold (full, v2.0/v2.1) mode. If appropriate data files will not be found the port will fail.  
Default switch strategy: AoG (full) -> AoG (SW) -> PS

* --aog_30  
Switches the port to Blake Stone: Aliens of Gold (full, v3.0) mode. If appropriate data files will not be found the port will fail.  
Default switch strategy: AoG (full) -> AoG (SW) -> PS

* --ps  
Switches the port to Blake Stone: Planet Strike (full, v1.0/v1.1) mode.  If appropriate data files will not be found the port will fail.  
Default switch strategy: AoG (full) -> AoG (SW) -> PS

* --no_screens  
Skips startup screens (AoG/PS) and ending promo pages (AoG SW only).

* --cheats  
Enables debug keys.

* --data_dir path_to_data  
Specifies location to the game's data files.  
Default: current working directory.

* --profile_dir path  
  Overrides default location of the game's profile files.  

* --vid_renderer [soft|ogl]  
Forces to use a specified renderer. "soft" selects a software renderer. "ogl" selects an OpenGL 2.x compatible renderer.  
Default order without this option: ogl, soft.

* --vid_windowed  
Runs the game in windowed mode.  
Default video mode: 640x480

* --vid_mode width height  
Selects the specified resolution for windowed mode.  Without this option the game will use desktop's resolution.  
Minimum width: 640  
Minimum height: 480

* --vid_scale factor  
Refinement factor. The higher a value the greater internal resolution mode will be used to render a scene. The dimensions of the resolution mode are proportional to the original one (320x200) by 'factor' value. This option can greatly affect the performance of a renderer (especially a software one).  
Minimum factor: 1 (identical to the original game)  
Default factor: depends on the game's resolution mode.

* --vid_window_x offset  
Sets a horizontal offset from the left side of the desktop screen.  
Applicable for windowed mode only.

* --vid_window_y offset  
Sets a vertical offset from the top side of the desktop screen.  
Applicable for windowed mode only.

* --snd_rate sampling_rate  
Specifies sampling rate of mixer in hertz.  
Default: 44100 Hz  
Minimum: 11025 Hz

* --snd_mix_size duration  
Specifies mix data size in milliseconds.  
Default: 40 ms  
Minimum: 20 ms


Cheat key
====

[J] [A] [M] [Enter]  
Press specified keys sequentially. Shows message "NOW you're jammin'!!", and gives to you all keys, all weapons and restores health to 100% but zeroes score points. Not available in shareware version.


Debug keys
===

Add option "--cheats" to enable these keys.

* [Backspace]+[A]  
Toggles visibility of actors on auto-map.

* [Backspace]+[C]  
Shows counts of total static objects, in use static objects, doors, total actors, active actors.

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
Each usage adds 99% health, 5 tokens, 50 ammo, one new weapon.

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
Warps to specified map but loads it from scratch rather from a saved game file/memory.

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


Compiling
===

Minimum requirements:
* C++11 compatible compiler.  

* CMake 2.8  
  (<http://cmake.org/>)  
  
* pkg-config  
  (<http://pkg-config.freedesktop.org/>)  
  (only for non Visual C++ compiler)  
  
* SDL v2.0.1 (non-Windows) / SDL v2.0.3 (Windows)  
  (<http://www.libsdl.org/>)  

Tested platforms:
  * Windows 10 x64  
    Visual C++ v19.10.25019 (Visual Studio 2017 Community Edition)  
    CMake x64 v3.9.0  
    SDL v2.0.5  
    
  * Ubuntu Desktop x64 16.04.4  
    GCC v5.4.0  
    CMake x64 v3.5.1  
    SDL v2.0.4  
  
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
  If enabled links modules statically to avoid dependency on  
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


Third party use
===

* [Simple DirectMedia Library](http://libsdl.org/) (v2)  
See file [COPYING-SDL2.txt](https://raw.githubusercontent.com/bibendovsky/bstone/master/COPYING-SDL2.txt) for a license information.

* [DOSBox](http://www.dosbox.com/)  
See file [dosbox/COPYING](https://raw.githubusercontent.com/bibendovsky/bstone/master/dosbox/COPYING) for a license information.  
Note: The source port uses only an OPL emulation code.


Credits
===

* [id Software](http://www.idsoftware.com/)  
Developing Wolfenstein 3D engine.

* [JAM Productions](http://wikipedia.org/wiki/JAM_Productions_%28software%29)  
Developing the game.

* [Apogee Entertainment, LLC](http://www.apogeesoftware.com/)  
Publishing the game and releasing a source code.

* Scott Smith  
Adaptation to Pandora console, various fixes.

* Filipe Tolhuizen  
Testing the port.


Links
===

* Apogee's [article](http://www.apogeesoftware.com/uncategorized/apogee-releases-blake-stone-source-code) about releasing of an original source code.


Disclaimer
===

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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

For a copy of the GNU General Public License see file LICENSE.  
For an original source code license see file ["Blake Stone source code license.doc"](https://github.com/bibendovsky/bstone/blob/master/Blake%20Stone%20source%20code%20license.doc?raw=true).
