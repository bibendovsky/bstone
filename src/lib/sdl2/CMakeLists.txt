#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_sdl2 VERSION 1.0.0 LANGUAGES NONE)

message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")

set(SDL2_DISABLE_UNINSTALL ON CACHE BOOL "Disable uninstallation of SDL2" FORCE)
set(SDL2_DISABLE_SDL2MAIN ON CACHE BOOL "Disable building/installation of SDL2main" FORCE)
set(SDL_SHARED OFF CACHE BOOL "Build a shared version of the library" FORCE)
set(SDL_STATIC ON CACHE BOOL "Build a static version of the library" FORCE)
set(SDL_TEST OFF CACHE BOOL "Build the SDL2_test library" FORCE)
set(SDL_TESTS OFF CACHE BOOL "Build the test directory" FORCE)

if (BSTONE_USE_STATIC_LINKING)
	set(SDL_FORCE_STATIC_VCRT ON CACHE BOOL "Force /MT for static VC runtimes" FORCE)
endif ()

add_subdirectory(src)
