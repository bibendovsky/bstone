bstone-vita
===========

Vita port of BStone. Allowing you to play Blake Stone natively on that handheld.

Installing and Running
======================

Place the game assets at the root of ux0:/data/bstone/ and install the .vpk

The full versions of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike can both currently be obatined from the 3D Realms website in addition to Steam and GOG (in every case for much less than $29.95!)

Planet Strike crashes frequently (a little less often without sound), so is not recommended at this point in time. Number keys, right quote, equals, and dash are hardcoded to the touch screen for easy weapon switching, elevator use, etc. Joystick sensitivity (and hence maximum speed) is subject to change, feedback welcome (if really needed, you can edit the config file to change it)

Compiling
=========

Install vitasdk and cmake, enter into the ./vita directory, and type

cmake .

followed by

cmake --build .

Credits
=======

Boris Bendovsky for BStone
JAM Productions, id Software, and Apogee for Blake Stone
The control functions were based on those written by fgsfdsfgs for the vita port of Chocolate Doom
The makers of vitasdk & Henkaku for making this possible in the first place
