bstone
======

A source port of Blake Stone: Planet Strike.


Contents
========

1. Disclaimer
2. Compiling
3. Command-line options
4. Third party use
5. Credits
6. Links


1 - Disclaimer
==============

Copyright (c) 1992-2013 Apogee Entertainment, LLC  
Copyright (c) 2013 Boris Bendovsky (<bibendovsky@hotmail.com>)

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


2 - Compiling
=============

The build system is based on CMake (<http://cmake.org/>).

Required libraries:

* SDL v2 or higher.


3 - Command-line options
========================

* --ren [soft|ogl]  
  Forces to use a specified renderer.  
  "soft" selects a software renderer.  
  "ogl" selects an Open GL 2.x compatible renderer.  
  Default order without this option: ogl, soft.

* --windowed  
  Runs the game in windowed mode.  
  Default resolution mode: 320x200

* --res width height  
  Selects the specified resolution for windowed mode.  
  Minimum width: 320  
  Minimum height: 200

* --winx offset  
  Sets a horizontal offset from the left side of the desktop screen.  
  Applicable only for windowed mode.

* --winy offset  
  Sets a vertical offset from the top side of the desktop screen.  
  Applicable only for windowed mode.


4 - Third party use
===================

* Simple DirectMedia Library (v2)  
  <http://libsdl.org/>  
  See file COPYING-SDL2.txt for a license information.

* DOSBox  
  <http://www.dosbox.com/>  
  See file dosbox/COPYING for a license information.  
  Note: The source port uses only an OPL emulation code.


5 - Credits
===========

* id Software  
  Developing Wolfenstein 3D engine.  
  Web-site: <http://www.idsoftware.com/>

* JAM Productions  
  Developing the game.

* Apogee Entertainment, LLC  
  Publishing the game and releasing a source code.  
  Web-site: <http://www.apogeesoftware.com/>

* Boris Bendovsky  
  Author of the source code.  
  Email: <bibendovsky@hotmail.com>

* Scott Smith  
  Adaption to Pandora console, various fixies.


6 - Links
=========

* The source port code and binary releases:  
  <https://github.com/bibendovsky/bstone/>

* Apogee's article about releasing of an original source code:  
  <http://www.apogeesoftware.com/uncategorized/apogee-releases-blake-stone-source-code>
