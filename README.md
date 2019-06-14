BStone
======

A source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike.


Contents
========

1. Disclaimer
2. Overview
   1. Overview (PS Vita)
3. Required assets
4. Profile
5. Compiling
6. Command-line options
7. Cheat key
8. Debug keys
9. Third party use
10. Credits
11. Links  
    1. General
    2. Add-ons for Aliens Of Gold (full)
    3. Add-ons for Planet Strike


1 - Disclaimer
==============

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013-2019 Boris I. Bendovsky (<bibendovsky@hotmail.com>)

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
For an original source code license see file "Blake Stone source code license.doc".


2 - Overview
============

BStone is a source port of Blake Stone game series: Aliens of Gold and Planet Strike.

Features:
* High resolution rendering of world (extended vanilla engine)
* Modern and vanilla controls
* Allows to customize control bindings
* Separate volume control of sound effects and music

Supported games:
* Aliens of Gold (v1.0/v2.0/v2.1/v3.0) full or shareware
* Planet Strike (v1.0/v1.1)


2.1 - Overview (PS Vita)
========================
See README-PSVITA.md for details about the source port on PS Vita.


3 - Required assets
===================

Since all titles are not free (except shareware) you have to own a copy of the game(s) in order to play.

Required files for each game:

|    AoG SW    |      AoG     |      PS      |
|--------------|--------------|--------------|
| AUDIOHED.BS1 | AUDIOHED.BS6 | AUDIOHED.VSI |
| AUDIOT.BS1   | AUDIOT.BS6   | AUDIOT.VSI   |
|              | EANIM.BS6    | EANIM.VSI    |
|              | GANIM.BS6    |              |
| IANIM.BS1    | IANIM.BS6    | IANIM.VSI    |
| MAPHEAD.BS1  | MAPHEAD.BS6  | MAPHEAD.VSI  |
| MAPTEMP.BS1  | MAPTEMP.BS6  | MAPTEMP.VSI  |
| SANIM.BS1    | SANIM.BS6    |              |
| VGADICT.BS1  | VGADICT.BS6  | VGADICT.VSI  |
| VGAGRAPH.BS1 | VGAGRAPH.BS6 | VGAGRAPH.VSI |
| VGAHEAD.BS1  | VGAHEAD.BS6  | VGAHEAD.VSI  |
| VSWAP.BS1    | VSWAP.BS6    | VSWAP.VSI    |

Legend:
* AoG SW - Aliens of Gold (shareware)
* AoG - Aliens of Gold (full)
* PS - Planet Strike


4 - Profile
===========

Configuration file, saved game files, etc. are stored in user's profile.  
The path to those files depends on platform.  
To override the path use --profile_dir option.

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


5 - Compiling
=============

Minimum requirements:
* C++14 compatible compiler.

* CMake 3.1.3  
  (<http://cmake.org/>)

* SDL v2.0.4  
  (<http://www.libsdl.org/>)

CMake variables:
* CMAKE_BUILD_TYPE  
  Selects which build(s) to compile.  
  Use semicolon to separate entries. 
  Usually it's Debug or Release.  
  For other values see CMake documentation.

* CMAKE_INSTALL_PREFIX  
  Selects location where install files to.

* BSTONE_PANDORA  
  Enables build for Open Pandora.

* BSTONE_USE_PCH  
  If enabled utilizes precompiled headers to speed up compilation.  
  Note: Visual C++ only

* BSTONE_USE_STATIC_LINKING  
  If enabled links modules statically to avoid dependency on  
  system and custom libraries at run-time.

* BSTONE_USE_MULTI_PROCESS_COMPILATION  
  Enables multi-process compilation if supported.

* SDL2W_SDL2_DIR  
  Defines directory with SDL2 CMake configuration file or with official SDL2 development Windows build.


Notes:
* Use ON value to enable option and value OFF to disable option.


6 - Command-line options
========================

* --version  
  Outputs the port's version to standard output and  
  into message box.

* --aog_sw  
  Switches the port to Blake Stone: Aliens of Gold (shareware) mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)

* --aog  
  Switches the port to Blake Stone: Aliens of Gold mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)
  
* --ps  
  Switches the port to Blake Stone: Planet Strike mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)

* --no_screens  
  Skips start-up screens and the ending promo pages (AoG SW).

* --cheats  
  Enables so called "debug mode" without much fuss.

* --data_dir path_to_data  
  Specifies location to the game's data files.  
  Default: current working directory.

* --mod_dir path_to_data  
  Specifies location to the mod's data files.  
  Default: undefined.

* --profile_dir path  
  Overrides default location of the game's profile files.  

* --vid_is_windowed  
  Runs the game in windowed mode.  
  Default video height: 480

* --vid_width width  
  Specifies window width.  
  Minimum width: 320  
  Default width: 640  

* --vid_height height  
  Specifies window height.  
  Minimum height: 240  
  Default height: 480

* --vid_no_vsync  
  Disables vertical synchronization.

* --vid_x offset  
  Sets a horizontal offset from the left side of the desktop screen.  
  Applicable for windowed mode only.

* --vid_y offset  
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


7 - Cheat key
=============

[J] [A] [M] [Enter]  
Press specified keys sequentially.  
Shows message "NOW you're jammin'!!", and gives to you all keys,  
all weapons and restores health to 100% but zeroes score points.  
Not available in shareware version.


8 - Debug keys
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


9 - Third party use
===================

* Simple DirectMedia Library  
  <http://libsdl.org/>  
  See file COPYING-SDL2.txt for a license information.

* DOSBox  
  <http://www.dosbox.com/>  
  See file dosbox/COPYING for a license information.  
  Note: The source port uses only an OPL emulation code.


10 - Credits
===========

* id Software  
  Developing Wolfenstein 3D engine.  
  <http://www.idsoftware.com/>

* JAM Productions  
  Developing the game.

* Apogee Entertainment, LLC  
  Publishing the game and releasing a source code.  
  <http://www.apogeesoftware.com/>

* Scott Smith  
  Adaptation to Pandora console, various fixes.

* Filipe Tolhuizen  
  Testing the port.


11 - Links
==========

11.1 - General
==============

* Home page:  
  <http://bibendovsky.github.io/bstone/>

* Blake Stone: Aliens of Gold official site:  
  <http://legacy.3drealms.com/blake/index.html>

* Blake Stone: Planet Strike official site:  
  <http://legacy.3drealms.com/planet/index.html>

* Apogee's article about releasing of an original source code:  
  <http://www.apogeesoftware.com/uncategorized/apogee-releases-blake-stone-source-code>

* Original source code:  
  <http://bibendovsky.github.io/bstone/files/official/Blake%20Stone%20Planet%20Strike%20Source%20Code.rar>

* Repacked shareware Blake Stone: Aliens Of Gold (v3.0):  
  <http://bibendovsky.github.io/bstone/files/official/repack/bs_aog_v3_0_sw.zip>

11.2 - Add-ons for Aliens Of Gold (full)
========================================

* Add-on "BSE90" by ack  
  <http://bibendovsky.github.io/bstone/files/community/aog/bse90.zip>

* Add-on "GUYSTONE" by Guy Brys  
  <http://bibendovsky.github.io/bstone/files/community/aog/guystone.zip>

* Ling's Blake Stone Levels by Ling Yan Li  
  <http://bibendovsky.github.io/bstone/files/community/aog/lingstone.zip>

11.3 - Add-ons for Planet Strike
================================

* Add-on "BSE24" by ack  
  <http://bibendovsky.github.io/bstone/files/community/ps/bse24.zip>

* Add-on "GUYSTRIKE" by Guy Brys  
  <http://bibendovsky.github.io/bstone/files/community/ps/guystrike.zip>

* Ling's Planet Strike Levels by Ling Yan Li  
  <http://bibendovsky.github.io/bstone/files/community/ps/lingstrike.zip>
