#[[

CMake wrapper for SDL2 module.


Copyright (c) 2018 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
OR OTHER DEALINGS IN THE SOFTWARE.


Virtual components:
    - static - uses static version of SDL2.

Required variables:
    - SDL2W_SDL2_DIR - the directory with SDL2 CMake configuration files or
                       the directory with official SDL2 development Windows build.

Targets:
    - SDL2W

]]


cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)

set(SDL2W_SDL2_DIR "" CACHE PATH "The directory with CMake configuration files or the directory with official SDL2 development Windows build. Leave empty to figure out the location of SDL2.")

set(SDL2_DIR "" CACHE PATH "The directory containing a CMake configuration file for SDL2." FORCE)

find_package(SDL2 QUIET HINTS ${SDL2W_SDL2_DIR})

unset(SDL2W_TMP_USE_STATIC)
set(SDL2W_TMP_VERSION_STRING "")

unset(SDL2W_TMP_NO_CMAKE_CONFIG)

if (NOT SDL2_FOUND AND SDL2W_SDL2_DIR)
	unset(SDL2W_TMP_ARCH_NAME)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(SDL2W_TMP_ARCH_NAME x64)
	elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set(SDL2W_TMP_ARCH_NAME x86)
	endif ()

	if (SDL2W_TMP_ARCH_NAME)
		set(SDL2W_TMP_SDL2_INCLUDE_DIR ${SDL2W_SDL2_DIR}/include)
		set(SDL2W_TMP_SDL2_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/lib/${SDL2W_TMP_ARCH_NAME})

		set(SDL2W_TMP_SDL_H ${SDL2W_TMP_SDL2_INCLUDE_DIR}/SDL.h)
		set(SDL2W_TMP_SDL2_LIB ${SDL2W_TMP_SDL2_LIBRARIES_DIR}/SDL2.lib)
		set(SDL2W_TMP_SDL2MAIN_LIB ${SDL2W_TMP_SDL2_LIBRARIES_DIR}/SDL2main.lib)

		if (EXISTS ${SDL2W_TMP_SDL_H} AND EXISTS ${SDL2W_TMP_SDL2_LIB} AND EXISTS ${SDL2W_TMP_SDL2MAIN_LIB})
			set(SDL2W_TMP_NO_CMAKE_CONFIG TRUE)
			set(SDL2_INCLUDE_DIRS ${SDL2W_TMP_SDL2_INCLUDE_DIR})
			set(SDL2_LIBRARIES ${SDL2W_TMP_SDL2_LIB};${SDL2W_TMP_SDL2MAIN_LIB})
		endif ()
	endif ()
endif ()

if (SDL2_FOUND OR SDL2W_TMP_NO_CMAKE_CONFIG)
	# Parse components.
	#
	foreach(SDL2W_TMP_COMP ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
		if (SDL2W_TMP_COMP STREQUAL "static")
			set(SDL2W_TMP_USE_STATIC TRUE)
		endif ()
	endforeach()

	# Get include directories.
	#
	unset(SDL2W_TMP_INCLUDE_DIRS)

	if (WIN32 AND NOT MINGW AND NOT SDL2W_TMP_NO_CMAKE_CONFIG)
		if (SDL2W_TMP_USE_STATIC)
			get_target_property(
				SDL2W_TMP_INCLUDE_DIRS
				SDL2::SDL2-static
				INTERFACE_INCLUDE_DIRECTORIES
			)
		else ()
			get_target_property(
				SDL2W_TMP_INCLUDE_DIRS
				SDL2::SDL2
				INTERFACE_INCLUDE_DIRECTORIES
			)
		endif ()
	else ()
		set(SDL2W_TMP_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
	endif ()

	# Find header file with version.
	#
	if (SDL2W_TMP_INCLUDE_DIRS)
		set(SDL2W_TMP_SDL_VERSION_H ${SDL2W_TMP_INCLUDE_DIRS}/SDL_version.h)

		# Convert POSIX path to Windows one (MinGW)
		#
		if (MINGW)
			execute_process(
				COMMAND "sh" "-c" "cmd //c echo ${SDL2W_TMP_SDL_VERSION_H}"
				TIMEOUT 7
				OUTPUT_VARIABLE SDL2W_TMP_SDL_VERSION_H
				ERROR_QUIET
				OUTPUT_STRIP_TRAILING_WHITESPACE
			)
		endif ()

		# Extract version.
		#
		if (EXISTS ${SDL2W_TMP_SDL_VERSION_H})
			set(
				SDL2W_TMP_MAJOR_REGEX
				"^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9])$"
			)

			set(
				SDL2W_TMP_MINOR_REGEX
				"^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9])$"
			)

			set(
				SDL2W_TMP_PATCH_REGEX
				"^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9])$"
			)

			file(
				STRINGS
				${SDL2W_TMP_SDL_VERSION_H}
				SDL2W_TMP_MAJOR_VERSION_STRING
				REGEX ${SDL2W_TMP_MAJOR_REGEX}
			)

			file(
				STRINGS
				${SDL2W_TMP_SDL_VERSION_H}
				SDL2W_TMP_MINOR_VERSION_STRING
				REGEX ${SDL2W_TMP_MINOR_REGEX}
			)

			file(
				STRINGS
				${SDL2W_TMP_SDL_VERSION_H}
				SDL2W_TMP_PATCH_VERSION_STRING
				REGEX ${SDL2W_TMP_PATCH_REGEX}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_MAJOR_REGEX}
				"\\1"
				SDL2W_TMP_MAJOR_VERSION
				${SDL2W_TMP_MAJOR_VERSION_STRING}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_MINOR_REGEX}
				"\\1"
				SDL2W_TMP_MINOR_VERSION
				${SDL2W_TMP_MINOR_VERSION_STRING}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_PATCH_REGEX}
				"\\1"
				SDL2W_TMP_PATCH_VERSION
				${SDL2W_TMP_PATCH_VERSION_STRING}
			)

			set(
				SDL2W_TMP_DIGIT_REGEX
				"^[0-9]$"
			)

			if (SDL2W_TMP_MAJOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
				SDL2W_TMP_MINOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
				SDL2W_TMP_PATCH_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX}
				)
				if (NOT ${SDL2W_TMP_MAJOR_VERSION} EQUAL 2)
					message(FATAL_ERROR "Unsupported major version (got: ${SDL2W_TMP_MAJOR_VERSION}; expected: 2).")
				endif ()

				set(
					SDL2W_TMP_VERSION_STRING
					${SDL2W_TMP_MAJOR_VERSION}.${SDL2W_TMP_MINOR_VERSION}.${SDL2W_TMP_PATCH_VERSION}
				)
			endif ()
		endif ()
	endif ()


	# Default handler.
	#
	include(FindPackageHandleStandardArgs)

	unset(SDL2W_TMP_REQUIRED_VARS)

	if (SDL2W_TMP_NO_CMAKE_CONFIG)
		set(SDL2W_TMP_REQUIRED_VARS SDL2W_TMP_NO_CMAKE_CONFIG)
	else ()
		set(SDL2W_TMP_REQUIRED_VARS SDL2_FOUND)
	endif ()

	find_package_handle_standard_args(
		${CMAKE_FIND_PACKAGE_NAME}
		REQUIRED_VARS
			${SDL2W_TMP_REQUIRED_VARS}
		VERSION_VAR
			SDL2W_TMP_VERSION_STRING
	)

	# Add target.
	#
	if (NOT TARGET ${CMAKE_FIND_PACKAGE_NAME})
		add_library(${CMAKE_FIND_PACKAGE_NAME} INTERFACE)

		if (NOT WIN32 OR MINGW OR SDL2W_TMP_NO_CMAKE_CONFIG)
			target_include_directories(
				${CMAKE_FIND_PACKAGE_NAME}
				INTERFACE
					${SDL2_INCLUDE_DIRS}
			)
		endif ()

		if (WIN32 AND NOT MINGW AND NOT SDL2W_TMP_NO_CMAKE_CONFIG)
			target_link_libraries(
				${CMAKE_FIND_PACKAGE_NAME}
				INTERFACE
					SDL2::SDL2main
			)
			
			if (SDL2W_TMP_USE_STATIC)
				target_link_libraries(
					${CMAKE_FIND_PACKAGE_NAME}
					INTERFACE
						SDL2::SDL2-static
				)
			else ()
				target_link_libraries(
					${CMAKE_FIND_PACKAGE_NAME}
					INTERFACE
						SDL2::SDL2
				)
			endif ()
		else ()
			target_link_libraries(
				${CMAKE_FIND_PACKAGE_NAME}
				INTERFACE
					${SDL2_LIBRARIES}
			)

			if (MINGW OR SDL2W_TMP_NO_CMAKE_CONFIG)
				target_link_libraries(
					${CMAKE_FIND_PACKAGE_NAME}
					INTERFACE
						imm32
						setupapi
						version
						winmm
				)
			endif ()
		endif ()
	endif ()
else ()
	if (${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
		message(FATAL_ERROR "SDL2 not found.")
	endif ()
endif ()
