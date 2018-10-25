## Issues:

### High Priority
* In elevator, fade out from viewport instead of messagebox
* Distance fade in AOG differs from DOS version (off by 1?)
* Tearing during fadein

### Medium Priority
* Crashes sporadically when music is enabled
* Premature partial fades to red on SW exit screens
* Make 4:3 mode work properly

### Low Priority
* Memory corruption and crashing after exiting using the PS button (or sleep?)
* Delay in sound playing when triggered for the first time
* Analog movement speed depends on framerate?
* Small seam in exact? middle of screen-- from "mirroring"?

## Future Features:

* Change cache logic, minimizing file access
* Fill numeric tables with higher accuracy values
* Rewrite routines to use cartesian instead of polar as much as possible
* Multithreading various drawing routines
* In-prompt buttons- use x,o
* In-game control customization for Vita
* Add option to disable loading screens

## Changes:

### 0.3

### 0.2
* L is now "use"; -/= in the front
* Joystick sesitivity adjusted
* Performance improvement (>20%)
* HUD no longer drawn over in various circumstances
* Live Area assets tweaked, -ps shortcut added
* Heartbeat on by default

### 0.1.1
* Elevator button touch location in ui_stretch mode

### 0.1
* Merge in changes from mainline develop branch up to the current state

### 0.0.1
* Initial port to PS Vita
