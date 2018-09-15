bstone-vita
===========

Vita port of BStone. Allowing you to play Blake Stone games natively on that handheld.

Installing and Running
======================

Install the `.vpk`.
Then, in the root of `ux0:/data/bstone/`, place the game assets, consisting of the following files:

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

Depending on which version(s) you wish to run.

The full versions of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike each can currently be obatined from the 3D Realms website, and additionally from Steam and GOG.

Planet Strike crashes frequently (a little less often without sound), so is not recommended at this point in time. Number keys, equals, and dash are hardcoded to the touch screen for easy weapon switching, elevator use, and map-zooming. Joystick sensitivity (and hence maximum speed) is subject to change in future versions, feedback welcome (if really needed, you can edit the config file to change it)

Compiling
=========

Install vitasdk and cmake, enter into the vita/ subdirectory, and type

```
cmake . && cmake --build .
```

Credits
=======

Boris Bendovsky for BStone

JAM Productions, id Software, and Apogee for Blake Stone

The control functions were based on those written by fgsfdsfgs for the vita port of Chocolate Doom

The makers of vitasdk & Henkaku for making this possible in the first place
