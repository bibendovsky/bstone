bstone-vita
===========

Port of the BStone sourceport of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike (https://bibendovsky.github.io/bstone/) to the Playstation Vita.

Installing and Running
======================

Install the `.vpk`.
Then, in the root of `ux0:/data/bstone/`, place the game assets, which consist, depending on the intended version, of the following files:

| Shareware    | Registered   | Planet Strike |
|--------------|--------------|---------------|
| AUDIOHED.BS1 | AUDIOHED.BS6 | AUDIOHED.VSI  |
| AUDIOT.BS1   | AUDIOT.BS6   | AUDIOT.VSI    | 
|              | EANIM.BS6    | EANIM.VSI     |
|              | GANIM.BS6    |               |
| IANIM.BS1    | IANIM.BS6    | IANIM.VSI     |
| MAPHEAD.BS1  | MAPHEAD.BS6  | MAPHEAD.VSI   |
| MAPTEMP.BS1  | MAPTEMP.BS6  | MAPTEMP.VSI   |
| SANIM.BS1    | SANIM.BS6    |               |
| VGADICT.BS1  | VGADICT.BS6  | VGADICT.VSI   |
| VGAGRAPH.BS1 | VGAGRAPH.BS6 | VGAGRAPH.VSI  |
| VGAHEAD.BS1  | VGAHEAD.BS6  | VGAHEAD.VSI   |
| VSWAP.BS1    | VSWAP.BS6    | VSWAP.VSI     |

The full versions of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike each can currently be obatined from the 3D Realms website, and additionally from Steam and GOG.

Any combination of these three may coexist in the directory. Press the main "start" region to launch the game. Game version will be chosen in the following order of preference: Registered, Shareware, Planet Strike. Press the "Planet Strike" region to start Planet Strike regardless of the presence of other versions.

Playing the Game
================

## Controls

L - use/open

R - fire/accept

× - about face

○ - select next available weapon/ back out of menu

△ - fire/accept

□ - use/open

↑ - move forward

↓ - move backward

← - turn left

→ - turn right

left stick - straight ahead and side-to-side motion

right stick - turn

Select - map or status window toggle

Start - Menu/back

Number keys are hard mapped to a column on the right side of the front touchscreen corresponding to the locations of the elevator buttons on the AOG level select screen. Useful for weapon selection in addition to operating the elevator. The left and right halves of the region occupied by the bottom HUD bar are mapped to "-", and "=", respectively, for controlling the map zoom level in Planet Strike. The left and right halves of the top region of the touchscreen select the previous/ next available weapon.

Special Notice
==============

The savegame format has been changed from the previous version, so previous saves are unfortunately incompatible with 0.3.
As with any homebrew Vita app, memory corruption may occur when returning from the LiveArea to an already running instance of the program. It is recommended, instead, to close and restart the app before resuming use.

Compiling
=========

Install vitasdk and cmake, enter into the `build/` subdirectory, and type

```
cmake ../src/vita/ && cmake --build .
```

Credits
=======

Boris Bendovsky for BStone

JAM Productions, id Software, and Apogee for Blake Stone

The control functions were based on those written by fgsfdsfgs for the vita port of Chocolate Doom

Rinnegatamante for help with various things

The makers of vitasdk & Henkaku for making this possible in the first place
