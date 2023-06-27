BStone
======

Unofficial source port for Blake Stone series (Aliens Of Gold and Planet Strike).


Contents
========

1. Disclaimer
2. Overview  
   2.1 Overview (PS Vita)
3. Known issues  
   3.1 Windows
4. Installation  
   4.1 Windows (GOG / Steam)  
   4.2 Windows (Generic)  
   4.3 Addons
5. Required assets
6. Profile
7. Audio  
   7.1 OpenAL driver
8. External textures  
   8.1 Aspect ratio  
   8.2 Transparency  
   8.3 Naming conventions  
   8.4 Supported file formats  
   8.5 File format search order
9. Taking screenshots
10. Compiling  
   10.1. Generic instructions for Linux-based system or build environment (MinGW)
11. Command-line options
12. Cheat key
13. Debug keys
14. Third party use
15. Credits
16. Links  
    16.1. Essentials  
    16.2. General  
    16.3. Add-ons for Aliens Of Gold (full)  
    16.4. Add-ons for Planet Strike


1 - Disclaimer
==============

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013-2022 Boris I. Bendovsky (<bibendovsky@hotmail.com>)

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

BStone is unofficial source port for "Blake Stone" game series: "Aliens Of Gold" and "Planet Strike".

Features:
* High resolution vanilla rendering
* 3D-rendering
* Upscale texture filter
* Support for external textures
* Allows to customize control bindings
* 3D-audio
* Separate volume control of sound effects and music

Supported games:
* Aliens Of Gold (v1.0/v2.0/v2.1/v3.0) full or shareware
* Planet Strike (v1.0/v1.1)


2.1 - Overview (PS Vita)
========================
See [README-PSVITA.md](README-PSVITA.md) for details about the source port on PS Vita.


3 - Known issues
================

3.1 - Windows
=============

*Problem:* OpenAL runtime v1.9.9 causes a (crash)[http://github.com/bibendovsky/bstone/issues/375] in non-MinGW-W64 build.  
*Solution:* Update the (OpenAL runtime)[http://openal.org/].


4 - Installation
================


4.1 - Windows (GOG / Steam)
===========================

- Download the latest release.
- Extract it into convinient for you directory.
- Run `bstone.exe` to play.


4.2 - Windows (generic)
=======================

- Download the latest release.
- Extract it into the directory with game files (*.BS1 / *.BS6 / *.VSI).
- Run `bstone.exe` to play.


4.3 - Addons
============

- Put addon's files into separate directory. Do not overwrite any original files!
- Run with command line option `--mod_dir` to point to this directory (i.e. `bstone.exe --mod_dir addon1`).


5 - Required assets
===================

Since all titles except shareware are not free you have to own a copy of the game in order to play.

Required files for each game:

|    AOG SW    |      AOG     |      PS      |
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

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG SW v1.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS1 | 0c3de403b524107809fa9308b730d60e8e41ba93 |
| AUDIOT.BS1   | 3d4e8b62aa4683671027513ad9720f76f176ba5b |
| IANIM.BS1    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS1  | 55b9eb9ed555b0f249c2cefd54ecc3f511bfcd55 |
| MAPTEMP.BS1  | 293464e7143ff7e6faf5f5b20799e76a394d65bf |
| SANIM.BS1    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS1  | b54d48d35d095e27a3c9130cfa59ed9c4f05abe7 |
| VGAGRAPH.BS1 | d1fa19131281d30787125f946881652be876f57a |
| VGAHEAD.BS1  | cc44c362e9e2c9f7b9fb2e4ba31331dde42a1e96 |
| VSWAP.BS1    | 37412ca9139562fc31330d72470822de4d9ceb4a |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG SW v2.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS1 | 0c3de403b524107809fa9308b730d60e8e41ba93 |
| AUDIOT.BS1   | 3d4e8b62aa4683671027513ad9720f76f176ba5b |
| IANIM.BS1    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS1  | 04a8e4e7c360f6033dc70f7b09edbc4a6447e462 |
| MAPTEMP.BS1  | bbb3cd6ab3e742eada427862504eba06437036d0 |
| SANIM.BS1    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS1  | 058fe0b59c7aa020bf4e7509103892d4c7459aa6 |
| VGAGRAPH.BS1 | e14b6172b6ab7568d3736f087f07a0df5eac5dad |
| VGAHEAD.BS1  | 6c0e273df7fd3940c038fc20a5bdfb81cd50e7ef |
| VSWAP.BS1    | dbde907ba1110bef445d6daae283c4520d6951b4 |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG SW v2.1:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS1 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS1   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| IANIM.BS1    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS1  | 04a8e4e7c360f6033dc70f7b09edbc4a6447e462 |
| MAPTEMP.BS1  | bbb3cd6ab3e742eada427862504eba06437036d0 |
| SANIM.BS1    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS1  | ed1ab61ad2529e046f966d9c1627508f76693ef8 |
| VGAGRAPH.BS1 | b59cbe3793b4612b06254ece48bf8e961ab6f528 |
| VGAHEAD.BS1  | 0635a4a2b823eef2904ed1d590d98362fb16621d |
| VSWAP.BS1    | dbde907ba1110bef445d6daae283c4520d6951b4 |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG SW v3.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS1 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS1   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| IANIM.BS1    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS1  | 04a8e4e7c360f6033dc70f7b09edbc4a6447e462 |
| MAPTEMP.BS1  | bbb3cd6ab3e742eada427862504eba06437036d0 |
| SANIM.BS1    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS1  | 537676ddcafeee415c22bb9d00097b25bc7e13c5 |
| VGAGRAPH.BS1 | 9f9f4c40c9637af472ce5e1a360e51364c3a418a |
| VGAHEAD.BS1  | e7fb8f241b8fa94739c3bd09ea05c1afc7bbfc95 |
| VSWAP.BS1    | dbde907ba1110bef445d6daae283c4520d6951b4 |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG v1.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS6 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS6   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| EANIM.BS6    | af5af59ad7ed17517d87b3ece3c0cac23ade535b |
| GANIM.BS6    | 9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e |
| IANIM.BS6    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS6  | 6f19a144d8985e90f096fc1c67ade58e9051235c |
| MAPTEMP.BS6  | 4d00c5f5c843f99a266bd938648192a6eff17b5a |
| SANIM.BS6    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS6  | d4f81ace1701a7338d43ce07723c2adaafdc837c |
| VGAGRAPH.BS6 | 02dc27d4810e3ffa26540b310eac6091f5e16de0 |
| VGAHEAD.BS6  | 639ec9e7a81ad83fc5b5c557cf4fc5fa28b9676b |
| VSWAP.BS6    | 0a700732ccbc72f95318a6226a7e1ad78ac713bb |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG v2.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS6 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS6   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| EANIM.BS6    | af5af59ad7ed17517d87b3ece3c0cac23ade535b |
| GANIM.BS6    | 9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e |
| IANIM.BS6    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS6  | 028f624e150f84ffc8336336cb0ecea0932cc22d |
| MAPTEMP.BS6  | 8b48e7eb859382a4c84948c5a62899194288e853 |
| SANIM.BS6    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS6  | e83b690836c9edf9ef60f6189b8384fb2319b735 |
| VGAGRAPH.BS6 | 67a679e3b107db8685ba5ff1643a38f9291b00bf |
| VGAHEAD.BS6  | 48b18caa86151610957b64b207cf2a2977ef7d57 |
| VSWAP.BS6    | 6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG v2.1:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS6 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS6   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| EANIM.BS6    | af5af59ad7ed17517d87b3ece3c0cac23ade535b |
| GANIM.BS6    | 9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e |
| IANIM.BS6    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS6  | 028f624e150f84ffc8336336cb0ecea0932cc22d |
| MAPTEMP.BS6  | 8b48e7eb859382a4c84948c5a62899194288e853 |
| SANIM.BS6    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS6  | e4ae3ef9a3ac158a832092b7b5487227337c6f13 |
| VGAGRAPH.BS6 | 6868e6cc4f8cb9160a218c5bce680a11f64c675a |
| VGAHEAD.BS6  | 1a3864cd12de4b8fd6be023b73d2d79e91b9018d |
| VSWAP.BS6    | 6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for AOG v3.0:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.BS6 | e2c101f9fd4bc7e22ddbfa3f019c9303877de4e2 |
| AUDIOT.BS6   | b79e7afcb4e4b29e59660ec261f72ff1ab4d3a25 |
| EANIM.BS6    | af5af59ad7ed17517d87b3ece3c0cac23ade535b |
| GANIM.BS6    | 9b2418a1a1f34abdcf6fcd9ed3344a49912c9b5e |
| IANIM.BS6    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.BS6  | 028f624e150f84ffc8336336cb0ecea0932cc22d |
| MAPTEMP.BS6  | 8b48e7eb859382a4c84948c5a62899194288e853 |
| SANIM.BS6    | 22bf818465da0f32eef9611de936cba9966b14aa |
| VGADICT.BS6  | 60da35e506d57753f83cca5f232a76dd1cf074ba |
| VGAGRAPH.BS6 | 001037fafdff124befb5437c563d0b9c613b3c00 |
| VGAHEAD.BS6  | ff165ca5924d738853587a7e4ab1cd239e10e359 |
| VSWAP.BS6    | 6fcc6e007b02b2e55892cfa7acfd42966ef9c9fb |

[SHA-1](https://en.wikipedia.org/wiki/SHA-1) for PS:

|   File name  |                  SHA-1                   |
|--------------|------------------------------------------|
| AUDIOHED.VSI | 2ed9a587926b2b455f8176788f595d03f6359171 |
| AUDIOT.VSI   | ecac8a800c5d021c8b68d170066c8859df9bd79a |
| EANIM.VSI    | b668330e56ceb069c1b972cefddd8a3d618a14af |
| IANIM.VSI    | ba1fa5b0bc34f148deffb4977a3cd1e718e91bbd |
| MAPHEAD.VSI  | c3fb0a9b81e0240d12c952fe5b57b78c1108aa48 |
| MAPTEMP.VSI  | acd03031d526eeaee64072518adf6814f96e2a37 |
| VGADICT.VSI  | 061692d166f68d7e0b81568725439078cc728f33 |
| VGAGRAPH.VSI | e619a3e6245f79888da5436df138d63204d2caba |
| VGAHEAD.VSI  | ad5fed9de5d1d82145df3ff2286e4bc62cb80e2a |
| VSWAP.VSI    | 4113ce83c42e69dc5cb20de79f41971a100f876e |

Legend:
* AOG SW - Aliens Of Gold (shareware)
* AOG - Aliens Of Gold (full)
* PS - Planet Strike

Both all lowercase and all uppercase (default) file names are supported.

The port auto-detect assets of some digital distribution services.
Note that option `--data_dir` disables auto-detection.

Supported services:

- [GOG](http://gog.com/)  
  Windows only.

- [Steam](http://store.steampowered.com)  
  Windows only.  


6 - Profile
===========

Configuration file, saved game files, etc. are stored in user's profile. The path to those files depends on platform. To override the path use `--profile_dir` option.

On Windows, the path might look like `C:\Users\john\AppData\Roaming\bibendovsky\bstone\`

On Linux, the path might look like `/home/john/.local/share/bibendovsky/bstone/`

On Mac OS X, the path might look like `/Users/john/Library/Application Support/bibendovsky/bstone/`

Configuration file name: `bstone_config`  
Log file name: `bstone_log.txt`  
High scores file name: `bstone_<game>_high_scores`  
Saved game file name: `bstone_<game>_saved_game`

Where `<game>` is:
* `aog_sw` - Aliens Of Gold (shareware)
* `aog_full` - Aliens Of Gold (full)
* `ps` - Planet Strike


7 - Audio
=========

Provides default stereo driver and optional 3D one.  
Use option `GAME OPTIONS - SOUND - DRIVER` to change the driver.


7.1 - OpenAL driver
===================

Supports any OpenAL v1.1 compatible implementation (Creative Labs, OpenAL Soft, etc.).  
Implementation driver should be available system wide (i.e., via `oalinst.exe`) or should be placed along with port's binary.  
Expected driver name is `OpenAL32.dll` on Windows or `libopenal.so` on non-Windows system.  
Set configuration string `snd_oal_library` to use another name.

The port uses default device name.  
Set configuration string `snd_oal_device_name` to open specific device.


8 - External textures
=====================

Allows to replace stocked textures with custom ones.

Supported targets: wall, sprite, flooring, ceiling.

Option `GAME OPTIONS - VIDEO - TEXTURING - EXTERNAL TEXTURES` enables or disables them on the fly.  
WARNING Changing the option may take some time if the dimensions of loaded textures are high.


8.1 - Aspect ratio
==================

Both vanilla games ran only in a 320x200 video mode. On monitors, widely available at the time, this video mode took up the entire screen, which had a 4:3 physical aspect ratio. This meant that the 320x200 display, with a 16:10 logical ratio, was stretched vertically - each pixel was 20% taller than it was wide.

In-game graphics have been specifically designed for the 320x200 resolution as stretched to a 4:3 physical aspect ratio.

The port stretches the geometry (world, objects) vertically by 20%.
That means if you created a wall or sprite texture, for example, with physical dimensions 1024x1024 it will be rendered as 1024x1228, i.e. stretched.

Flooring and ceiling textures has always 1:1 aspect ratio.

TLDR
- Design wall, sprite, HUD element or screen images in 1:1.2 aspect ratio (i.e. 1280x1536), but export for the game in 1:1 aspect ratio (i.e. 1024x1024).
- Design flooring or ceiling image in 1:1 aspect ratio (i.e. 1280x1280), and export for the game in 1:1 aspect ratio (i.e. 1024x1024) too.


8.2 - Transparency
==================

Images with alpha channel should be exported as [*premultiplied*](http://en.wikipedia.org/wiki/Alpha_compositing).


8.3 - Naming conventions
========================

All letters *should be* lower case.

"Aliens Of Gold" resources are goes into `aog` directory.
"Planet Strike" resources are goes into `ps` directory.

Sprite name format: `sprite_xxxxxxxx.ext`.
Wall name format: `wall_xxxxxxxx.ext`.

Where:
- `xxxxxxxx` - an internal decimal number of the sprite padded with zero on the left. For "Aliens Of Gold" those internal numbers are based on version `2.1`.
- `.ext` - file name extension.

Examples:
- `aog/sprite_00000034.bmp` - "Aliens Of Gold" yellow key card sprite in BMP format.
- `aog/wall_00000088.png` - "Aliens Of Gold" turned on south-north switch wall in PNG format.


8.4 - Supported file formats
============================

- [Windows BMP](http://wikipedia.org/wiki/BMP_file_format)
- [PNG](http://wikipedia.org/wiki/Portable_Network_Graphics)


8.5 -  File format search order
===============================

1. PNG
2. BMP


9 - Taking screenshots
======================

Default key is <kbd>F5</kbd>.  
Use menu to modify bindings.

Taken screenshots are placed in the profile directory.

Supported format: [PNG](http://wikipedia.org/wiki/Portable_Network_Graphics)


10 - Compiling
=============

Minimum requirements:

* C++14 compatible compiler.
  * [GCC 5.0](http://gcc.gnu.org)
  * [Clang 3.4](http://clang.llvm.org)
  * [MSVC (VS 2017 15.0)](http://visualstudio.microsoft.com)
* [CMake 3.4.0](http://cmake.org/)
* [SDL v2.0.4](http://libsdl.org/)

CMake variables:
* `CMAKE_BUILD_TYPE`  
  Selects which build(s) to compile. Use semicolon to separate entries. Usually it's `Release`. For other values see CMake documentation.

* `CMAKE_INSTALL_PREFIX`  
  Selects location where to install files to.

* `BSTONE_PANDORA`  
  Enables build for Open Pandora.

* `BSTONE_USE_PCH`  
  If enabled utilizes precompiled headers to speed up the compilation.

* `BSTONE_USE_STATIC_LINKING`  
  If enabled links modules statically to avoid dependency on system and custom libraries at run-time.

* `BSTONE_USE_MULTI_PROCESS_COMPILATION`  
  Enables multi-process compilation if supported.

* `BSTONE_MORE_COMPILER_WARNINGS`  
  Enables more compilation warnings.

* `BSTONE_TESTS`  
  Builds an application to test internal facility.  
  Default value: OFF

* `SDL2W_SDL2_DIR`  
  Defines directory with SDL2 CMake configuration file or with official SDL2 development Windows build.

Notes:
* Use `ON` value to enable option and value `OFF` to disable option.


10.1 - Generic instructions for Linux-based system or build environment (MinGW)
=============================================================================

1. Install minimum required software described above.

2. Obtain source code and extract it (if necessary) into some directory. For our example we will use working directory `~/bstone-x.y.z`.

3. Make directory `~/bstone-x.y.z/build` current.

4. Generate Makefile.  
`cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=~/bstone-x.y.z/build/install ..`

5. Build and install:
`cmake --build . --target install`

6. On success you will find executable and text files in the directory `~/bstone-x.y.z/build/install`.


11 - Command-line options
========================

* `--version`  
  Outputs the port's version to standard output and into message box.

* `--aog_sw`  
  Switches the port to "Blake Stone: Aliens Of Gold (shareware)" mode.  
  If appropriate data files will not be found the port will fail.

* `--aog`  
  Switches the port to "Blake Stone: Aliens Of Gold" mode.  
  If appropriate data files will not be found the port will fail.

* `--ps`  
  Switches the port to "Blake Stone: Planet Strike" mode.  
  If appropriate data files will not be found the port will fail.

* `--no_screens`  
  Skips start-up screens and the ending promo pages (AOG SW).

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
    - `auto_detect` - tries to select the best renderer.
    - `software` - the vanilla renderer.
    - `gl_2_0` - OpenGL 2.0 or higher.
    - `gl_3_2_c` - OpenGL 3.2 core or higher.
    - `gles_2_0` - OpenGL ES 2.0 or higher.

  Default: `auto_detect`

* `--vid_width width`  
  Specifies window width.  
  Minimum width: `320`  
  Default width: `640`

* `--vid_height height`  
  Specifies window height.  
  Minimum height: `240`  
  Default height: `480`

* `--vid_x offset`  
  Sets a horizontal offset from the left side of the desktop screen.  
  Applicable for positionable window only.  
  Default: `0`

* `--vid_y offset`  
  Sets a vertical offset from the top side of the desktop screen.  
  Applicable for positionable window only.  
  Default: `0`

* `--vid_is_positioned value`  
  Centers a window on the desktop or moves it in the specified position.  
  Values: `0` (centered) or `1` (positioned)  
  Default: `0`

* `--vid_is_vsync value`  
  Enables or disables vertical synchronization.  
  Values: `0` (disable) or `1` (enable)  
  Default: `1`

* `--vid_is_widescreen value`  
  Enables or disables widescreen rendering.  
  Values: `0` (disable) or `1` (enable)  
  Default: `1`

* `--vid_is_ui_stretched value`  
  Stretches the UI or keeps it at 4x3 ratio otherwise.  
  Values: `0` (non-stretched) or `1` (stretched)  
  Default: `0`

* `--vid_2d_texture_filter filter`  
  Sets texturing filter for UI.  
  Applicable only for 3D-rendering.  
  Values: `nearest` or `linear`  
  Default: `nearest`

* `--vid_3d_texture_image_filter filter`  
  Sets texturing image filter for the scene.  
  Applicable only for 3D-rendering.  
  Values: `nearest` or `linear`  
  Default: `nearest`

* `--vid_3d_texture_mipmap_filter filter`  
  Sets texturing mipmap filter for the scene.  
  Applicable only for 3D-rendering.  
  Values: `nearest` or `linear`  
  Default: `nearest`

* `--vid_3d_texture_anisotropy value`  
  Sets anisotropy degree for the scene.  
  Value `1` or lower disables the filter.  
  Applicable only for 3D-rendering.  
  Values: [`1`..`16`]  
  Default: `1`

* `--vid_texture_upscale_filter filter`  
  Sets texturing upscale filter.  
  Applicable only for 3D-rendering.  
  Values: `none` or `xbrz`  
  Default: `none`  
  **WARNING** `xbrz` is a high resource usage filter!

* `--vid_texture_upscale_xbrz_degree degree`  
  Sets a degree of xBRZ texturing upscale.  
  Applicable only for 3D-rendering.  
  Values: [`2`..`6`]  
  Default: `0`

* `--vid_aa_kind value`  
  Sets an anti-aliasing mode.  
  Applicable only for 3D-rendering.  
  Values: `none` or `msaa`  
  Default: `none`

* `--vid_aa_degree value`  
  Sets a degree of the anti-aliasing.  
  Applicable only for 3D-rendering.  
  Values: [`2`..`32`]  
  Default: `1`

* `--vid_filler_color_index value`  
  Sets a color for screen bars.  
  Values: [`0`..`255`]  
  Default: `0`

* `--vid_external_textures value`  
  Toggles external textures.  
  Values: `0` (disable), `1` (enable).  
  Default: `0`

* `--snd_is_disabled value`  
  Enables or disables audio subsystem.  
  Values: `0` (disable) or `1` (enable)  
  Default: `0`

* `--snd_rate sampling_rate`  
  Specifies sampling rate of mixer in hertz.  
  Default: `44100`  
  Minimum: `11025`

* `--snd_mix_size duration`  
  Specifies mix data size in milliseconds.  
  Default: `40`  
  Minimum: `20`

* `--snd_driver value`  
  Specifies the audio driver to use.  
  Values: `auto-detect`, `system` (system default), `openal` (OpenAL)  
  Default: `auto-detect`  
  Auto-detect order: `openal`, `system`

* `--snd_oal_library value`  
  Specifies OpenAL driver's name.  
  Default: "" (`OpenAL32.dll` on Windows and `libopenal.so` on non-Windows system).

* `--snd_oal_device_name value`  
  Specifies OpenAL device name.  
  Default: ""

* `--snd_sfx_type value`  
  Specifies SFX type.  
  Values: `adlib` (AdLib) or `pc_speaker` (PC Speaker)  
  Default: `adlib`

* `--snd_is_sfx_digitized value`  
  Toggles SFX digitization.  
  If enabled overrides AdLib / PC Speaker SFX audio chunk if such one is available in `AUDIOT.*` file.  
  Values: `0` (disable) or `1` (enable)  
  Default: `1`

* `--snd_opl3_type value`  
  Specifies OPL3 emulator type.  
  Values: `dbopl` (DOSBox) or `nuked` (Nuked)  
  Default: `dbopl`

* `--calculate_hashes`  
  Calculates hashes (SHA-1) of all resource files and outputs them into the log.

* `--extract_vga_palette dir`  
  Extracts VGA palette into existing directory `dir`.  
  Supported file format: [BMP](http://wikipedia.org/wiki/BMP_file_format)

* `--extract_walls dir`  
  Extracts graphics resources (wall, flooring, etc.) into existing directory `dir`.  
  Supported file format: [BMP](http://wikipedia.org/wiki/BMP_file_format)

* `--extract_sprites dir`  
  Extracts graphics resources (actors, decorations, etc.) into existing directory `dir`.  
  Supported file format: [BMP](http://wikipedia.org/wiki/BMP_file_format)

* `--extract_music dir`  
  Extracts music resources into existing directory `dir`.  
  Supported file format: [WAV](http://en.wikipedia.org/wiki/WAV)  
  Supported file format: data (unprocessed)

* `--extract_sfx dir`  
  Extracts sfx resources into existing directory `dir`.  
  Supported file format: [WAV](http://en.wikipedia.org/wiki/WAV)  
  Supported file format: data (unprocessed)

* `--extract_texts dir`  
  Extracts text resources into existing directory `dir`.  
  Supported file format: TXT

* `--extract_all dir`  
  Extracts all resources (walls, sprites, etc.) into existing directory `dir`.


12 - Cheat key
==============

<kbd>J</kbd> <kbd>A</kbd> <kbd>M</kbd> <kbd>Enter</kbd>  
Press specified keys sequentially. Shows message "NOW you're jammin'!!", and gives to you all keys, all weapons and restores health to 100% but zeroes score points. Not available in shareware version.


13 - Debug keys
===============

Add option `--cheats` to enable these keys.

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
  Warps to specified map but loads it from scratch rather from a saved game file or memory.

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

* <kbd>Backspace</kbd>+<kbd>l</kbd>  
  Dumps information into the log about remaining bonus items and enemies.


14 - Third party use
====================

* [SDL (Simple DirectMedia Library)](http://libsdl.org/)  
  See file `src/lib/sdl/COPYING.txt` for license information.

* [DOSBox](http://dosbox.com/)  
  See file `src/dosbox/COPYING` for license information.  
  Note: The port uses OPL emulation code only.

* [OpenAL Soft](http://openal-soft.org/)  
  See file `src/lib/openal_soft/COPYING` for license information.  
  Note: The port uses public headers only.

* [stb_image / stb_image_write](http://github.com/nothings/stb)  
  See file `src/lib/stb/LICENSE` for license information.

* [xBRZ](http://sourceforge.net/projects/xbrz/)  
  See directory `src/lib/xbrz` for license information (`License.txt`) and essential changes made for the port (`bstone_changelog.txt`).

* [Nuked OPL3](http://github.com/nukeykt/Nuked-OPL3)  
  See file `src/lib/nuked_opl3/LICENSE` for license information


15 - Credits
============

* [id Software](http://www.idsoftware.com/)  
  Wolfenstein 3D engine.  

* [JAM Productions](http://wikipedia.org/wiki/JAM_Productions_%28company%29)  
  The game itself.

* [Apogee Entertainment, LLC](http://www.apogeesoftware.com/)  
  Publishing the game and releasing a source code.  

* Scott Smith  
  Adaptation to Pandora console, various fixes.

* Filipe Tolhuizen  
  Testing the port.

* Various contributors for providing fixies, ideas, etc.


16 - Links
==========


16.1 - Essentials
=================

* [Home page](http://bibendovsky.github.io/bstone/)
* [Precompiled binaries and their source code](http://github.com/bibendovsky/bstone/releases)


16.1 - General
==============

* [Blake Stone: Aliens Of Gold official site](http://legacy.3drealms.com/blake/index.html)
* [Blake Stone: Planet Strike official site](http://legacy.3drealms.com/planet/index.html)
* [Apogee's article about releasing of an original source code](http://www.apogeesoftware.com/uncategorized/apogee-releases-blake-stone-source-code)
* [Original source code](http://bibendovsky.github.io/bstone/files/official/Blake%20Stone%20Planet%20Strike%20Source%20Code.rar)
* [Repacked shareware Blake Stone: Aliens Of Gold (v3.0)](http://bibendovsky.github.io/bstone/files/official/repack/bs_aog_v3_0_sw.zip)


16.2 - Add-ons for Aliens Of Gold (full)
========================================

* Add-on [BSE90](http://bibendovsky.github.io/bstone/files/community/aog/bse90.zip) by ack
* Add-on [GUYSTONE](http://bibendovsky.github.io/bstone/files/community/aog/guystone.zip) by Guy Brys
* Ling's Blake Stone [Levels](http://bibendovsky.github.io/bstone/files/community/aog/lingstone.zip) by Ling Yan Li


16.3 - Add-ons for Planet Strike
================================

* Add-on [BSE24](http://bibendovsky.github.io/bstone/files/community/ps/bse24.zip) by ack
* Add-on [GUYSTRIKE](http://bibendovsky.github.io/bstone/files/community/ps/guystrike.zip) by Guy Brys
* Ling's Planet Strike [Levels](http://bibendovsky.github.io/bstone/files/community/ps/lingstrike.zip) by Ling Yan Li
