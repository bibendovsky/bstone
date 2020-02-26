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
Copyright (c) 2013-2020 Boris I. Bendovsky (<bibendovsky@hotmail.com>)

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
* 3D-rendering
* Allows to customize control bindings
* Separate volume control of sound effects and music

Supported games:
* Aliens of Gold (v1.0/v2.0/v2.1/v3.0) full or shareware
* Planet Strike (v1.0/v1.1)


2.1 - Overview (PS Vita)
========================
See [README-PSVITA.md](README-PSVITA.md) for details about the source port on PS Vita.


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
`"C:\\Users\\john\\AppData\\Roaming\\bibendovsky\\bstone\\"`

On Linux, the path might look like:  
`"/home/john/.local/share/bibendovsky/bstone/"`

On Mac OS X, the path might look like:  
`"/Users/john/Library/Application Support/bibendovsky/bstone/"`

Configuration file name: `bstone_config`  
Log file name: `bstone_log.txt`  
High scores file name: `bstone_game_high_scores`  
Saved game file name: `bstone_game_saved_game`

Where "game" is:  
* `aog_sw` - Aliens of Gold (shareware)  
* `aog_full` - Aliens of Gold (full)  
* `ps` - Planet Strike


5 - Compiling
=============

Minimum requirements:
* C++14 compatible compiler.

* CMake 3.1.3  
  (<http://cmake.org/>)

* SDL v2.0.4  
  (<http://www.libsdl.org/>)

CMake variables:
* `CMAKE_BUILD_TYPE`  
  Selects which build(s) to compile.  
  Use semicolon to separate entries. 
  Usually it's `Debug` or `Release`.  
  For other values see CMake documentation.

* `CMAKE_INSTALL_PREFIX`  
  Selects location where install files to.

* `BSTONE_PANDORA`  
  Enables build for Open Pandora.

* `BSTONE_USE_PCH`  
  If enabled utilizes precompiled headers to speed up compilation.  
  Note: Visual C++ only

* `BSTONE_USE_STATIC_LINKING`  
  If enabled links modules statically to avoid dependency on  
  system and custom libraries at run-time.

* `BSTONE_USE_MULTI_PROCESS_COMPILATION`  
  Enables multi-process compilation if supported.

* `SDL2W_SDL2_DIR`  
  Defines directory with SDL2 CMake configuration file or with official SDL2 development Windows build.


Notes:
* Use `ON` value to enable option and value `OFF` to disable option.


6 - Command-line options
========================

* `--version`  
  Outputs the port's version to standard output and  
  into message box.  

* `--aog_sw`  
  Switches the port to Blake Stone: Aliens of Gold (shareware) mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)  

* `--aog`  
  Switches the port to Blake Stone: Aliens of Gold mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)  
  
* `--ps`  
  Switches the port to Blake Stone: Planet Strike mode.  
  If appropriate data files will not be found the port will fail.  
  Default switch strategy: AoG -> PS -> AoG (SW)  

* `--no_screens`  
  Skips start-up screens and the ending promo pages (AoG SW).  

* `--cheats`  
  Enables so called "debug mode" without much fuss.  

* `--data_dir dir`  
  Specifies a directory with game's resource files.  
  Default: current working directory.  

* `--mod_dir dir`  
  Specifies a directory with mod's resource files.  
  Default: undefined.  

* `--profile_dir dir`  
  Overrides default directory of the game's profile files.  

* `--vid_renderer value`  
  Select a renderer.  
  Values:
    - `auto_detect` - tries to auto-detect the best renderer.
    - `software` - the vanilla renderer.
    - `gl_2_0` - OpenGL 2.0 or higher.
    - `gl_3_2_c` - OpenGL 3.2. core.
    - `gles_2_0` - OpenGL ES 2.0 or higher.

  Default: `auto_detect`

* `--vid_is_windowed value`  
  Starts up the game in windowed mode (1) or in fake fullscreen (0) otherwise.  
  Values: 0, 1  
  Default: 1  

* `--vid_windowed_width width`  
  Specifies window width for windowed mode.  
  Minimum width: 320  
  Default width: 640  

* `--vid_windowed_height height`  
  Specifies window height for windowed mode.  
  Minimum height: 240  
  Default height: 480  

* `--vid_windowed_x offset`  
  Sets a horizontal offset from the left side of the desktop screen.  
  Applicable for positionable window only.  
  Default: 0  

* `--vid_windowed_y offset`  
  Sets a vertical offset from the top side of the desktop screen.  
  Applicable for positionable window only.  
  Default: 0  

* `--vid_is_positioned value`  
  Centers (0) a windowed window on the desktop or moves it (1) in the specified position.  
  Values: 0, 1  
  Default: 0  

* `--vid_is_vsync value`  
  Enables (1) or disables (0) vertical synchronization.  
  Values: 0, 1  
  Default: 1  

* `--vid_is_widescreen value`  
  Enables (1) or disables (0) widescreen rendering.  
  Values: 0, 1  
  Default: 1  

* `--vid_is_ui_stretched value`  
  Stretches (1) the UI or keeps it (0) at 4x3 ratio otherwise.  
  Values: 0, 1  
  Default: 0  

* `--vid_2d_texture_filter filter`  
  Sets texturing filter for UI.  
  Applicable only for 3D-rendering.  
  Values: `nearest`, `linear`  
  Default: `nearest`  

* `--vid_3d_texture_image_filter filter`  
  Sets texturing image filter for the scene.  
  Applicable only for 3D-rendering.  
  Values: `nearest`, `linear`  
  Default: `nearest`  

* `--vid_3d_texture_mipmap_filter filter`  
  Sets texturing mipmap filter for the scene.  
  Applicable only for 3D-rendering.  
  Values: `nearest`, `linear`  
  Default: `nearest`  

* `--vid_3d_texture_anisotropy value`  
  Sets anisotropy degree for the scene.  
  Value `1` or lower disables the filter.  
  Applicable only for 3D-rendering.  
  Values: [1..16]  
  Default: 1  

* `--vid_texture_upscale_filter filter`  
  Sets texturing upscale filter.  
  Applicable only for 3D-rendering.  
  Values: `none`, `xbrz`  
  Default: `none`  
  **WARNING** `xbrz` is a high resource usage filter!  

* `--vid_texture_upscale_xbrz_degree degree`  
  Sets a degree of xBRZ texturing upscale.  
  Applicable only for 3D-rendering.  
  Values: [2..6]  
  Default: 0  

* `--vid_aa_kind value`  
  Sets an anti-aliasing mode.  
  Applicable only for 3D-rendering.  
  Values: `none`, `msaa`  
  Default: `none`  

* `--vid_aa_degree value`  
  Sets a degree of the anti-aliasing.  
  Applicable only for 3D-rendering.  
  Values: [2..32]  
  Default: 1  

* `--snd_is_disabled value`  
  Enables (1) or disables (0) audio subsystem.  
  Default: 0  

* --snd_rate sampling_rate  
  Specifies sampling rate of mixer in hertz.  
  Default: 44100 Hz  
  Minimum: 11025 Hz  

* `--snd_mix_size duration`  
  Specifies mix data size in milliseconds.  
  Default: 40 ms  
  Minimum: 20 ms  

* `--debug_dump_hashes`  
  Dumps hashes (SHA-1) of all resource files into the log.  

* `--debug_dump_walls_images dir`  
  Dumps graphics resources (wall, flooring, etc.) into existing directory `dir`.  
  Supported file format: BMP (Windows)  

* `--debug_dump_sprites_images dir`  
  Dumps graphics resources (actors, decorations, etc.) into existing directory `dir`.  
  Supported file format: BMP (Windows)  


7 - Cheat key
=============

<kbd>J</kbd> <kbd>A</kbd> <kbd>M</kbd> <kbd>Enter</kbd>  
Press specified keys sequentially.  
Shows message "NOW you're jammin'!!", and gives to you all keys,  
all weapons and restores health to 100% but zeroes score points.  
Not available in shareware version.


8 - Debug keys
==============

Add option "--cheats" to enable these keys.

* <kbd>Backspace</kbd>+<kbd>A</kbd>  
  Toggles visibility of actors on auto-map.

* <kbd>Backspace</kbd>+<kbd>C</kbd>  
  Shows counts of total static objects, in use static objects,  
  doors, total actors, active actors.

* <kbd>Backspace</kbd>+<kbd>D</kbd>  
  Toggles player's invisibility.

* <kbd>Backspace</kbd>+<kbd>E</kbd>  
  Win mission instantly.

* <kbd>Backspace</kbd>+<kbd>F</kbd>  
  Shows player's coordinates and direction.

* <kbd>Backspace</kbd>+<kbd>G</kbd>  
  Toggles god mode.

* <kbd>Backspace</kbd>+<kbd>H</kbd>  
  Hurt yourself by 1%.

* <kbd>Backspace</kbd>+<kbd>I</kbd>  
  Each usage adds 99% health, 5 tokens, 50 ammo, one new weapon.

* <kbd>Backspace</kbd>+<kbd>K</kbd>  
  Shows total counts of enemies, points and informants on the map.

* <kbd>Backspace</kbd>+<kbd>M</kbd>  
  Memory information.

* <kbd>Backspace</kbd>+<kbd>O</kbd>  
  Shows push walls on auto-map.

* <kbd>Backspace</kbd>+<kbd>Q</kbd>  
  Instant quit.

* <kbd>Backspace</kbd>+<kbd>R</kbd>  
  Shows full map.

* <kbd>Backspace</kbd>+<kbd>S</kbd>  
  Slow motion.

* <kbd>Backspace</kbd>+<kbd>U</kbd>  
  Unlocks all maps.

* <kbd>Backspace</kbd>+<kbd>W</kbd>  
  Warps to specified map.

* <kbd>Backspace</kbd>+<kbd>Shift</kbd>/<kbd>Caps Lock</kbd>+<kbd>W</kbd>  
  Warps to specified map but loads it from scratch  
  rather from a saved game file/memory.

* <kbd>Backspace</kbd>+<kbd>Home</kbd>  
  Selects previous ceiling texture.

* <kbd>Backspace</kbd>+<kbd>Page Up</kbd>  
  Selects next ceiling texture.

* <kbd>Backspace</kbd>+<kbd>End</kbd>  
  Selects previous flooring texture.

* <kbd>Backspace</kbd>+<kbd>Page Down</kbd>  
  Selects next flooring texture.

* <kbd>Backspace</kbd>+<kbd>-</kbd>  
  Decreases shading depth.

* <kbd>Backspace</kbd>+<kbd>=</kbd>  
  Increases shading depth.

* <kbd>Backspace</kbd>+<kbd>[</kbd>  
  Decreases shading drop off.

* <kbd>Backspace</kbd>+<kbd>]</kbd>  
  Increases shading drop off.


9 - Third party use
===================

* Simple DirectMedia Library  
  <http://libsdl.org/>  
  See file `COPYING-SDL2.txt` for license information.

* DOSBox  
  <http://www.dosbox.com/>  
  See file `src/dosbox/COPYING` for license information.  
  Note: The source port uses only an OPL emulation code.

* GLM  
  <https://glm.g-truc.net/>
  See file `src/lib/glm/copying.txt` for license information.

* xBRZ  
  <http://sourceforge.net/projects/xbrz/>  
  See directory `src/lib/xbrz` for license information (`License.txt`) and essential changes made by the port (`bstone_changelog.txt`).  


10 - Credits
============

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
