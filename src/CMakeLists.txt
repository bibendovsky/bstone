#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)

add_subdirectory(lib/khronos)
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
