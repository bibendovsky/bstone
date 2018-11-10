#[[

CMake find module wrapper for SDL2.


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
    - SDL2_DIR - directory with import targets.

Targets:
    - SDL2W

]]


cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)

find_package(SDL2 QUIET)

unset(SDL2W_TMP_USE_STATIC)

set(SDL2W_TMP_VERSION_STRING "")

if (SDL2_FOUND)
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

	if (WIN32 AND NOT MINGW)
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
				SDL2W_MAJOR_VERSION_LINE
				REGEX ${SDL2W_TMP_MAJOR_REGEX}
			)

			file(
				STRINGS
				${SDL2W_TMP_SDL_VERSION_H}
				SDL2W_MINOR_VERSION_LINE
				REGEX ${SDL2W_TMP_MINOR_REGEX}
			)

			file(
				STRINGS
				${SDL2W_TMP_SDL_VERSION_H}
				SDL2W_PATCH_VERSION_LINE
				REGEX ${SDL2W_TMP_PATCH_REGEX}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_MAJOR_REGEX}
				"\\1"
				SDL2W_MAJOR_VERSION
				${SDL2W_MAJOR_VERSION_LINE}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_MINOR_REGEX}
				"\\1"
				SDL2W_MINOR_VERSION
				${SDL2W_MINOR_VERSION_LINE}
			)

			string(
				REGEX REPLACE
				${SDL2W_TMP_PATCH_REGEX}
				"\\1"
				SDL2W_PATCH_VERSION
				${SDL2W_PATCH_VERSION_LINE}
			)

			set(
				SDL2W_TMP_DIGIT_REGEX
				"^[0-9]$"
			)

			if (SDL2W_MAJOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
				SDL2W_MINOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
				SDL2W_PATCH_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX}
				)
				if (NOT ${SDL2W_MAJOR_VERSION} EQUAL 2)
					message(FATAL_ERROR "Unsupported major version (got: ${SDL2W_MAJOR_VERSION}; expected: 2).")
				endif ()

				set(
					SDL2W_TMP_VERSION_STRING
					${SDL2W_MAJOR_VERSION}.${SDL2W_MINOR_VERSION}.${SDL2W_PATCH_VERSION}
				)
			endif ()
		endif ()
	endif ()

	
	# Default handler.
	#
	include(FindPackageHandleStandardArgs)

	find_package_handle_standard_args(
		${CMAKE_FIND_PACKAGE_NAME}
		REQUIRED_VARS
			SDL2_FOUND
		VERSION_VAR
			SDL2W_TMP_VERSION_STRING
	)

	# Add target.
	#
	if (NOT TARGET ${CMAKE_FIND_PACKAGE_NAME})
		add_library(${CMAKE_FIND_PACKAGE_NAME} INTERFACE)

		if (NOT WIN32 OR MINGW)
			target_include_directories(
				${CMAKE_FIND_PACKAGE_NAME}
				INTERFACE
					${SDL2_INCLUDE_DIRS}
			)
		endif ()

		if (WIN32 AND NOT MINGW)
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

			if (MINGW)
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
