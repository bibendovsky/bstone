## Issues:

* -/= on front
* clear screen before elevator
* Crashes:
	* Planet Strike: playing multiple sounds when music enabled
	* General: saving/loading, changing levels, fadeout
* Image displayed after fadeout in level transitions?

Medium Priority:
*******
* Analog movement speed depends on framerate?
* Distance fade in AOG differs from DOS version

lower priority:
****
* Memory corruption and crashing after exiting using the PS button (or sleep?)
* Delay in sound playing when triggered for the first time
* Small seam in exact? middle of screen-- from "mirroring"?

## Future Features:

* Fill numeric tables with higher accuracy values
* Multithreading
* Adjust controls
* Pre-cache all assets/ Ramdisk
* Add screenshots
* In-prompt buttons

## Changes:

### 0.2
* L is now "use"; -/= in the front
* Joystick sesitivity adjusted
* Performance improvement (>20%)
* HUD no longer drawn over in various circumstances
* Live Area assets
* Heartbeat on by default

### 0.1.1
* Elevator button touch location in ui_stretch mode

### 0.1
* Merge in changes from mainline develop branch up to the current state

### 0.0.1
* Initial port to PS Vita