# BStone changelog

## [1.1.14] - 2019-11-09
### Added
- [Vita] Version 0.3 (by 01y).
- [Vita] Joystick sensitivity via CONTROLS menu (by 01y).
- [Vita] Previous/Next weapon controls (by 01y).
- [Vita] Circle button backs out of menu (by 01y).

### Changed
- README monospace formatting, keyboard glyphs (by cxong).
- [Vita] Heap size increased to 192 MiB (by 01y).
- [Vita] Assets search strategy (by 01y).
- [Vita] Control constants to match default keyboard speed (by 01y).

### Removed
- <kbd>Alt</kbd> and <kbd>Ctrl</kbd> to handle menu item (by bibendovsky).

### Fixed
- Text typos (by mnhauke).
- SDL with two digit sub-version number (by bibendovsky).
- Apply floor bonus correctly on percentage greater than 100% (by XxMiltenXx).
- [Vita] Readme corrections (by 01y).
- [Vita] Circle button behavior in menu (by 01y).
- [Vita] Sound volume visualization (by 01y).


## [1.1.13] - 2019-07-11
### Fixed
- [AOG] Allow to warp to level beyond the 10th one.
- Critical error if switch connected to non-activable object.


## [1.1.12] - 2019-02-09
### Fixed
- [CMake] Dependency on TestBigEndian.
- Dependencies on raycaster for hitscan, item pickup and saved game.

### Added
- [MSVC] Add option for multi-process compilation (BSTONE_USE_MULTI_PROCESS_COMPILATION).


## [1.1.11] - 2019-01-12
### Fixed
- Disabled fade in/out effect.
- Segfault at JAM logo on some platforms.
- Invalid direction of running Blake at the ending of mission 2.
- Lowered by one pixel lower status panel.
- Option "--profile_dir".


## [1.1.10] - 2018-12-31
### Added
- Allow lowercase asset file name.
- Allow to back to the game from the main menu by hitting ESC.
- Option to toggle UI stretching.
- Option to toggle intro/outro/promo.
- Playstation Vita support.
- Command-line option to dump assets SHA1 hash (--debug_dump_hashes).
- Command-line option to specify mod assets (--mod_dir).
- Track map file hash in saved game.
- AoG Shareware support (v1.0/v2.0/v2.1).
- Option to toggle fade in/out effect.

### Changed
- Rewrite CMake script.
- C++ standard to C++14.
- Revise code.
- Find assets strategy: AoG -> PS -> AoG SW

### Removed
- Command-line options: --aog_10, --aog_2x, --aog_30.

### Fixed
- One pixel gap between the edge of view and weapon sprite.
- Lightning on AoG maps.
- Mask UI between some screen transitions.


## [1.1.9] - 2017-09-08
### Fixed
- Teleporting out effect.


## [1.1.8] - 2017-08-06
### Added
- Widescreen mode.
- Icon and it's resource file (Windows).

### Changed
- Use text format for configuration.
- Made E2M6 completable with 100% stats.

### Removed
- Makefile.pandora.
- Custom renderers.

### Fixed
- Bottom view bound when drawing sprites.
- Command-line to force a software renderer.
- Mouse sensitivity.
- Weapon switching.
- Offset of floor info on upper status bar.


## [1.1.7] - 2016-04-09
### Fixed
- Interaction checks for doors.


## [1.1.6] - 2016-02-07
### Added
- Installation rules for CMake.

### Changed
- Do not play "use" sound if not interacting with bio-tech, wall or push-wall.

### Fixed
- Always unlock floor with red card after the last unlocked one.
- Crash in some cases for interaction code.
- Number of episodes for Planet Strike.
- Do not shade cloaked shapes.


## [1.1.5] - 2015-10-12
### Changed
- Increased max mouse sensitivity value.
    
### Fixed
- Option --version now works without data files.


## [1.1.4] - 2015-09-11
### Added
- Option to override default profile's path.


## [1.1.3] - 2015-08-29
### Fixed
- Falling back to software rendering.


## [1.1.2] - 2015-08-22
### Fixed
- (AOG) Discovered hidden area shows with darker color on map.
- (AOG) Weapon's image size on screen reduced a bit.


## [1.1.1] - 2015-06-22
### Fixed
- (AoG) Weapon scale.

## [1.1.0] - 2015-06-11
### Added
- Support of Aliens of Gold v1.0/v2.0/v3.0.
- Option to stretch rendered image to window.
- Handle game closing by pressing window's "close" button.

### Changed
- Improved audio and video performance.
- Disabled weapon bobbing in AoG.

### Fixed
- Palette updating for software rendering.
- Viewport for software rendering.
- Barriers operating on another floor.
- Barriers operating message.
- Player's gun hit scan.
- Crates with green and gold keys.
- Genetic Guard death sound.
- Push wall crash on E3M2 at (16:50).
- Uncleaned input state on gaining/loosing input focus.
- Missing toggle heartbeat sounds by key.
- Crash by a dummy switch on E2M1 at (35:61).
- Star Trooper death and faint sounds.
- Indestructible Projection Generators on E6M9.
- Total enemy count on E6M9.
- Paused audio on inactive/minimized window.
- Music toggle by key when enabling official cheat (J+A+M+Enter).


## [1.0.0] - 2015-05-17
### Added
- Supports Aliens of Gold (full & shareware).

### Changed
- All user specific files (configuration, saved games, log, .etc) now stored in user's profile.
- High scores stored in separate file.
- Configuration file shared among all games.
- All options renamed to reflect subsystem it belongs.
- Removed compatibility vanilla options.


## [0.9.5] - 2014-11-30
### Fixed
- Missing music on a new game start.
- Default configuration.


## [0.9.4] - 2014-11-17
### Added
- Two controls scheme: classic (original) and modern (WSAD-like).
- Allow to rebind any action.
- Allow circle strafing with mouse.
- 'Always run' option.
- Allow to control annoying wall hit sound.

### Fixed
- Message about malfunction weapon.
- Interrogation of 'mean' bio-tech.
- Overflow in sintable.


## [0.9.3] - 2014-07-01
### Added
- High resolution rendering (HRR).
- Command-line option "scale" to manual control of HRR degree.
- Option to toggle wall hit sound.

### Fixed
- Drawing of "vertical" push-walls.
- Stutters on item pickup.
- Sprite's rotation angle.


## [0.9.2] - 2014-06-11
### Fixed
- Crash in area 12 (approx. coordinates 33:29).


## [0.9.1] - 2014-06-06
### Added
- Support of keypad keys.


## [0.9.0] - 2014-04-08
### First release.
