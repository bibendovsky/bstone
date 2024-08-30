cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

add_subdirectory(lib/khronos)
add_subdirectory(lib/nuked_opl3)
add_subdirectory(lib/openal_soft)
add_subdirectory(lib/stb)
add_subdirectory(lib/xbrz)

if (BSTONE_INTERNAL_SDL2)
	add_subdirectory(lib/sdl2)
endif ()

add_subdirectory(tools)
add_subdirectory(bstone)

if (BSTONE_TESTS)
	add_subdirectory(tests)
endif ()
