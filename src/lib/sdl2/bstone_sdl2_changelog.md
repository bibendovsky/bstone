# BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
# SDL2 changelog

### Removed
- Directory *test*.

### Changed
- Disable dynamic API in *src/dynapi/SDL_dynapi.h*.
- Minimum CMake version to v3.5.0.
- [CMake] Fix block "if(MSVC AND NOT SDL_LIBC)..." to allow only static library.
