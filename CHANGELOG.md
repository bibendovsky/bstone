# BStone changelog


## [1.2.11] - 2021-03-21
### Added
- [CMake] Root CMakeLists.txt.
- [CMake] Add option `BSTONE_MORE_COMPILER_WARNINGS` for more compiler warnings.
- Bindings to take screenshots (default <kbd>F5</kbd>).
- Option to control weapon bobbing (actual for Planet Strike only).
- PC Speaker SFX.
- Option `snd_sfx_type` to switch between AdLib and PC Speaker audio.
- Option `snd_is_sfx_digitized` to toggle SFX digitization.
- Extracting unprocessed audio (*.data).
- Extracting PC Speaker audio.

### Changed
- [CMake] Minimum supported version is 3.4.0.
- Audio low-pass filtering is disabled by default.

### Fixed
- Compiling under non-MSYS MinGW.
- Content search.
- Show dropped important items (keys, etc.) on auto-map.
- Don't draw the scene for "show full map" cheat.
- Reduce compiling warnings.
- Make the application DPI aware.
- [OAL] Discard sounds with lower priority on same actor's channel.
- [OAL] Fix hangs on changing audio settings when any world's sound playing.
- [HWR] Mismatched texture for door track.
- [HWR] Flooring / Ceiling texture direction.
- Non-readable log file while application is running.
- Item flags for message box.


## [1.2.10] - 2021-02-14
### Added
- Support for external textures (bmp, png).
- Configuration for external textures (`vid_external_textures`).
- OpenAL 3D audio driver.
- Configuration for OpenAL driver (`snd_driver`, `snd_oal_library`, `snd_oal_device_name`).

### Fixed
- Search path with provided `data_dir`.
- Double fading on new game or load game when using hw renderer (Emile Belanger, @emileb).
- Fade whole screen when in widescreen and 3D view is showing (Emile Belanger, @emileb).
- Next map number for level transition.
- Support for lowercase resource file names.
- [CMake] Always enabled precompiled headers.


## [1.2.9] - 2020-11-14
### Added
- Option `vid_filler_color_index` to select filler color.
- [Windows] File information resource.

### Fixed
- Enable sfx and music by default for first startup.
- Lives counter.
- Input floor panel visual glitches.


## [1.2.8] - 2020-07-10
### Fixed
- Disabled episodes 5 and 6 in static build.


## [1.2.7] - 2020-07-07
### Fixed
- Extraction of compressed texts.
- Same texture for door type BIO.
- Player radius for item pickup.
- Missing test for pushwall in CheckLine.
- Fix sfx turning off when in-game sounds paused.
- Show option "Game Options -> Video -> Stretch UI".

### Changed
- Replaced almost all fixed-point arithmetic with a floating-point one.
- Relay on CMake's PCH support if available.


## [1.2.6] - 2020-06-18
### Added
- Option `snd_resampling_interpolation` to specify interpolation method for resampling.
- Option `snd_resampling_lpf` to toggle low-pass filter for resampling.


## [1.2.5] - 2020-06-11
### Fixed
- [AOG][E4L1] Removed unnecessary in-place level fix.


## [1.2.4] - 2020-06-11
### Added
- [AOG] "Cursor" for elevetor panel.
- Option `--extract_vga_palette` to extract VGA palette.
- Pause game audio when in menu.

### Changed
- Increased maximum actor count.
- Use floating point for audio mixing.
- Rewrite low-pass filter for PCM upsampling.
- Saved games are not compatible with previous version.

### Fixed
- [AOG] Revised initial hitpoints for bosses.
- [AOG][E4L1] Added missing barrier switch for 100% points.
- [PS] Hide already pushed pushwall on auto-map.
- Pickup items while attacking.
- Sprite precache for Perscan Drone and Volatile Material Transport.
- Both games should be now completable with 100% rating.


## [1.2.3] - 2020-05-27
### Added
- Input bindings to cycle weapons.

### Changed
- Apply bonus queue after finishing the mission.
- Write data into temporary file and then rename it.
- Enable high score menu for debug build.
- Saved games are not compatible with previous version.

### Fixed
- [AOG] Animation timings for some actors to match origin.
- [AOG] Bio-tech placed on special tile (E5L2; (18, 43)).
- Sprite precache for crate items.
- Sprite precache for morphed Dr. Goldfire.
- Level state after death when extra live is available.
- Add points for destroying projector generator.
- Displaying pushwalls after loading from saved game.
- Hit points table.
- Player's attack tracing.
- Zigzag movement when strafing along the wall.

### Removed
- Default values for some input bindings.


## [1.2.2] - 2020-05-14
### Fixed
- Handling of file paths.


## [1.2.1] - 2020-05-11
### Added
- Auto-detect game content of GOG and Steam.
- Show message box with options if multiple game contents found.


## [1.2.0] - 2020-05-09
### Added
- 3D-renderer (OpenGL backend).
- Menu to control video mode.
- Menu to control texturing.
- [RHW] Texture upscale filter (xBRZ).
- Option `snd_is_disabled` to ignore audio subsystem.
- Option `vid_is_positioned` to control position of the window.
- Option `vid_is_widescreen` to control widescreen mode.
- Option `vid_is_ui_stretched` to control UI stretching.
- Option `vid_2d_texture_filter` to control texturing filter for UI.
- Option `vid_3d_texture_image_filter` to control texturing image filter for the scene.
- Option `vid_3d_texture_mipmap_filter` to control texturing mipmap filter for the scene.
- Option `vid_3d_texture_anisotropy` to control anisotropy filter for the scene.
- Option `vid_texture_upscale_filter` to select texturing upscale filter.
- Option `vid_texture_upscale_xbrz_degree` to control degree of xBRZ filter.
- Option `vid_aa_kind` to select anti-aliasing mode.
- Option `vid_aa_degree` to select anti-aliasing degree.
- Option `calculate_hashes` to calculate hashes of resource files.
- Option `extract_walls` to extract graphics resources (wall, flooring, etc.).
- Option `extract_sprites` to extract graphics resources (actors, decorations, etc.).
- Option `extract_music` to extract music resources.
- Option `extract_sfx` to extract sfx resources.
- Option `extract_texts` to extract text resources.
- Option `extract_all` to extract all resources.
- Generic instruction on how to compile on Linux-based system.

### Removed
- Option `vid_scale`.
- Vanilla input bindings and related menu items.

### Changed
- Renamed option `vid_windowed` to `vid_is_windowed`.
- Renamed option `vid_window_x` to `vid_x`.
- Renamed option `vid_window_y` to `vid_y`.
- Renamed option `debug_dump_hashes` to `calculate_hashes`.
- Split option `vid_mode` into `vid_width` and `vid_height`.
- Renamed option `vid_no_vsync` to `vid_vsync`.
- Values of option `vid_renderer`.
- Serializing barrier switches.
- Disabled music caching.
- Reduced volume factor of OPL sounds.
- Saved games are not compatible with previous versions.

### Fixed
- Audio normalization.


## [1.1.16] - 2020-02-01
### Fixed
- Removed some checks on barriers (by bibendovsky).


## [1.1.15] - 2020-01-25
### Changed
- Number secret floor (by bibendovsky).

### Fixed
- Destination position for inter-floor teleporter (by bibendovsky).


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
