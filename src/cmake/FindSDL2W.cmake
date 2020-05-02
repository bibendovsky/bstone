#[[

CMake wrapper for SDL2 module.


Copyright (c) 2018-2020 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors.

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
                       the directory with official SDL2 development build for Visual C++.
                       Leave empty to search automatically.

Targets:
    - SDL2W::SDL2Wmain - replacement for `main` entry point.
    - SDL2W::SDL2W - SDL2.

]]


cmake_minimum_required (VERSION 3.1.3 FATAL_ERROR)

set (SDL2W_VERSION "1.0.2")
message (STATUS "[SDL2W] Version: ${SDL2W_VERSION}")

set (SDL2W_SDL2_DIR "" CACHE PATH "The directory with CMake configuration files or the directory with official SDL2 development Windows build. Leave empty to figure out the location of SDL2.")

set (SDL2_DIR "" CACHE PATH "The directory containing a CMake configuration file for SDL2." FORCE)

find_package (SDL2 QUIET HINTS ${SDL2W_SDL2_DIR})

set (SDL2W_TMP_FOUND_CONFIG FALSE)
set (SDL2W_TMP_FOUND_TARGETS FALSE)
set (SDL2W_TMP_FOUND_STATIC_TARGET FALSE)
set (SDL2W_TMP_TARGET "${CMAKE_FIND_PACKAGE_NAME}::${CMAKE_FIND_PACKAGE_NAME}")
set (SDL2W_TMP_TARGET_MAIN "${SDL2W_TMP_TARGET}main")

if (SDL2_FOUND)
	message (STATUS "[SDL2W] Found config.")

	set (SDL2W_TMP_FOUND_CONFIG TRUE)

	if (TARGET SDL2::SDL2main AND (TARGET SDL2::SDL2 OR TARGET SDL2::SDL2-static))
		message (STATUS "[SDL2W] Found targets.")

		set (SDL2W_TMP_FOUND_TARGETS TRUE)

		if (TARGET SDL2::SDL2-static)
			set (SDL2W_TMP_FOUND_STATIC_TARGET TRUE)
		endif ()
	elseif (SDL2_INCLUDE_DIRS AND SDL2_LIBRARIES)
		message (STATUS "[SDL2W] Found config variables.")
	else ()
		message (FATAL_ERROR "[SDL2W] Supported config not found.")
	endif ()
else ()
	message (STATUS "[SDL2W] Config not found.")
endif ()

#
# Official development build for Visual C++.
#
set (SDL2W_TMP_FOUND_MSVC_DEV FALSE)

if (MSVC AND NOT SDL2W_TMP_FOUND_CONFIG AND SDL2W_SDL2_DIR)
	if (CMAKE_SIZEOF_VOID_P EQUAL 8)
		set (SDL2W_TMP_ARCH_NAME x64)
	elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
		set (SDL2W_TMP_ARCH_NAME x86)
	else ()
		message (FATAL_ERROR "[SDL2W] Unsupported CPU architecture.")
	endif ()

	if (SDL2W_TMP_ARCH_NAME)
		set (SDL2W_TMP_SDL2_INCLUDE_DIR ${SDL2W_SDL2_DIR}/include)
		set (SDL2W_TMP_SDL2_LIBRARIES_DIR ${SDL2W_SDL2_DIR}/lib/${SDL2W_TMP_ARCH_NAME})

		set (SDL2W_TMP_SDL_H ${SDL2W_TMP_SDL2_INCLUDE_DIR}/SDL.h)
		set (SDL2W_TMP_SDL2_LIB ${SDL2W_TMP_SDL2_LIBRARIES_DIR}/SDL2.lib)
		set (SDL2W_TMP_SDL2MAIN_LIB ${SDL2W_TMP_SDL2_LIBRARIES_DIR}/SDL2main.lib)

		if (EXISTS ${SDL2W_TMP_SDL_H} AND EXISTS ${SDL2W_TMP_SDL2_LIB} AND EXISTS ${SDL2W_TMP_SDL2MAIN_LIB})
			message (STATUS "[SDL2W] Found official development build for Visual C++.")

			set (SDL2W_TMP_FOUND_MSVC_DEV TRUE)
			set (SDL2_INCLUDE_DIRS ${SDL2W_TMP_SDL2_INCLUDE_DIR})
			set (SDL2_LIBRARIES ${SDL2W_TMP_SDL2_LIB};${SDL2W_TMP_SDL2MAIN_LIB})
		endif ()
	endif ()
endif ()


set (SDL2W_TMP_VERSION_STRING "")
set (SDL2W_TMP_USE_STATIC FALSE)
set (SDL2W_TMP_SDL2_LIB_LOCATION "")
set (SDL2W_TMP_SDL2_STATIC_LIB_LOCATION "")
set (SDL2W_TMP_SDL2MAIN_LIB_LOCATION "")

if (SDL2_FOUND OR SDL2W_TMP_FOUND_MSVC_DEV)
	# Parse components.
	#
	if (${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS)
		message (STATUS "[SDL2W] Selected components:")

		foreach (SDL2W_TMP_COMP ${${CMAKE_FIND_PACKAGE_NAME}_FIND_COMPONENTS})
			message (STATUS "[SDL2W]     \"${SDL2W_TMP_COMP}\"")

			if (SDL2W_TMP_COMP STREQUAL "static")
				set (SDL2W_TMP_USE_STATIC TRUE)
			endif ()
		endforeach ()
	else ()
		message (STATUS "[SDL2W] No components were selected.")
	endif ()

	# Get targets location.
	#
	message (STATUS "[SDL2W] Targets location:")

	if (SDL2W_TMP_FOUND_TARGETS)
		# SDL2
		#
		set (SDL2W_TMP_SDL2_LOCATION "")

		if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_FOUND_STATIC_TARGET)
			get_target_property (
				SDL2W_TMP_SDL2_LOCATION
				SDL2::SDL2-static
				LOCATION
			)
		else ()
			get_target_property (
				SDL2W_TMP_SDL2_LOCATION
				SDL2::SDL2
				LOCATION
			)
		endif ()

		message (STATUS "[SDL2W]     \"${SDL2W_TMP_SDL2_LOCATION}\"")


		# SDL2main
		#
		set (SDL2W_TMP_SDL2_MAIN_LOCATION "")

		get_target_property (
			SDL2W_TMP_SDL2_MAIN_LOCATION
			SDL2::SDL2main
			LOCATION
		)

		message (STATUS "[SDL2W]     \"${SDL2W_TMP_SDL2_MAIN_LOCATION}\"")
	else ()
		foreach (SDL2W_TMP_LOCATION IN LISTS SDL2_LIBRARIES)
			message (STATUS "[SDL2W]     \"${SDL2W_TMP_LOCATION}\"")

			string (FIND ${SDL2W_TMP_LOCATION} "SDL2-static" SDL2W_TMP_SDL2_STATIC_POS REVERSE)
			string (FIND ${SDL2W_TMP_LOCATION} "SDL2main" SDL2W_TMP_SDL2MAIN_POS REVERSE)
			string (FIND ${SDL2W_TMP_LOCATION} "SDL2" SDL2W_TMP_SDL2_POS REVERSE)

			if (NOT (SDL2W_TMP_SDL2_STATIC_POS LESS 0))
				set (SDL2W_TMP_SDL2_STATIC_LIB_LOCATION ${SDL2W_TMP_LOCATION})
			elseif (NOT (SDL2W_TMP_SDL2MAIN_POS LESS 0))
				set (SDL2W_TMP_SDL2MAIN_LIB_LOCATION ${SDL2W_TMP_LOCATION})
			elseif (NOT (SDL2W_TMP_SDL2_POS LESS 0))
				set (SDL2W_TMP_SDL2_LIB_LOCATION ${SDL2W_TMP_LOCATION})
			endif ()
		endforeach ()
	endif ()

	# Get include directories.
	#
	if (SDL2W_TMP_FOUND_TARGETS)
		if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_FOUND_STATIC_TARGET)
			get_target_property (
				SDL2W_TMP_INCLUDE_DIRS
				SDL2::SDL2-static
				INTERFACE_INCLUDE_DIRECTORIES
			)
		else ()
			get_target_property (
				SDL2W_TMP_INCLUDE_DIRS
				SDL2::SDL2
				INTERFACE_INCLUDE_DIRECTORIES
			)
		endif ()
	else ()
		set (SDL2W_TMP_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
	endif ()

	if (NOT SDL2W_TMP_INCLUDE_DIRS)
		message (FATAL_ERROR "[SDL2W] Include directories not found.")
	endif ()


	# Find header file with version.
	#
	if (SDL2W_TMP_INCLUDE_DIRS)
		message (STATUS "[SDL2W] Include directories:")

		foreach (SDL2W_TMP_INCLUDE_DIR IN LISTS SDL2W_TMP_INCLUDE_DIRS)
			message (STATUS "[SDL2W]    \"${SDL2W_TMP_INCLUDE_DIR}\"")
		endforeach ()

		foreach (SDL2W_TMP_INCLUDE_DIR IN LISTS SDL2W_TMP_INCLUDE_DIRS)
			set (SDL2W_TMP_SDL_VERSION_H ${SDL2W_TMP_INCLUDE_DIR}/SDL_version.h)

			# Convert POSIX path to Windows one (MinGW)
			#
			if (MINGW)
				execute_process (
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
				set (
					SDL2W_TMP_MAJOR_REGEX
					"^#define[ \t]+SDL_MAJOR_VERSION[ \t]+([0-9]+)$"
				)

				set (
					SDL2W_TMP_MINOR_REGEX
					"^#define[ \t]+SDL_MINOR_VERSION[ \t]+([0-9]+)$"
				)

				set (
					SDL2W_TMP_PATCH_REGEX
					"^#define[ \t]+SDL_PATCHLEVEL[ \t]+([0-9]+)$"
				)

				file (
					STRINGS
					${SDL2W_TMP_SDL_VERSION_H}
					SDL2W_TMP_MAJOR_VERSION_STRING
					REGEX ${SDL2W_TMP_MAJOR_REGEX}
				)

				file (
					STRINGS
					${SDL2W_TMP_SDL_VERSION_H}
					SDL2W_TMP_MINOR_VERSION_STRING
					REGEX ${SDL2W_TMP_MINOR_REGEX}
				)

				file (
					STRINGS
					${SDL2W_TMP_SDL_VERSION_H}
					SDL2W_TMP_PATCH_VERSION_STRING
					REGEX ${SDL2W_TMP_PATCH_REGEX}
				)

				string (
					REGEX REPLACE
					${SDL2W_TMP_MAJOR_REGEX}
					"\\1"
					SDL2W_TMP_MAJOR_VERSION
					${SDL2W_TMP_MAJOR_VERSION_STRING}
				)

				string (
					REGEX REPLACE
					${SDL2W_TMP_MINOR_REGEX}
					"\\1"
					SDL2W_TMP_MINOR_VERSION
					${SDL2W_TMP_MINOR_VERSION_STRING}
				)

				string (
					REGEX REPLACE
					${SDL2W_TMP_PATCH_REGEX}
					"\\1"
					SDL2W_TMP_PATCH_VERSION
					${SDL2W_TMP_PATCH_VERSION_STRING}
				)

				set (
					SDL2W_TMP_DIGIT_REGEX
					"^[0-9]+$"
				)

				if (SDL2W_TMP_MAJOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
					SDL2W_TMP_MINOR_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX} AND
					SDL2W_TMP_PATCH_VERSION MATCHES ${SDL2W_TMP_DIGIT_REGEX}
					)
					if (NOT ${SDL2W_TMP_MAJOR_VERSION} EQUAL 2)
						message (FATAL_ERROR "[SDL2W] Unsupported major version (got: ${SDL2W_TMP_MAJOR_VERSION}; expected: 2).")
					endif ()

					set (
						SDL2W_TMP_VERSION_STRING
						${SDL2W_TMP_MAJOR_VERSION}.${SDL2W_TMP_MINOR_VERSION}.${SDL2W_TMP_PATCH_VERSION}
					)
				endif ()

				break ()
			endif ()
		endforeach ()
	else ()
		message (STATUS "[SDL2W] No include directories.")
	endif ()

	message (STATUS "[SDL2W] Found SDL version: ${SDL2W_TMP_VERSION_STRING}")


	# Default handler.
	#
	include (FindPackageHandleStandardArgs)

	set (SDL2W_TMP_REQUIRED_VARS "")

	if (SDL2W_TMP_FOUND_TARGETS)
		set (SDL2W_TMP_REQUIRED_VARS SDL2_FOUND)
	else ()
		set (SDL2W_TMP_REQUIRED_VARS SDL2_INCLUDE_DIRS;SDL2_LIBRARIES)
	endif ()

	find_package_handle_standard_args (
		${CMAKE_FIND_PACKAGE_NAME}
		REQUIRED_VARS
			${SDL2W_TMP_REQUIRED_VARS}
		VERSION_VAR
			SDL2W_TMP_VERSION_STRING
	)

	# Add target.
	#
	if (NOT TARGET ${SDL2W_TMP_TARGET})
		if (SDL2W_TMP_FOUND_TARGETS)
			add_library (${SDL2W_TMP_TARGET} INTERFACE IMPORTED)

			if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_FOUND_STATIC_TARGET)
				target_link_libraries (
					${SDL2W_TMP_TARGET}
					INTERFACE
						SDL2::SDL2-static
				)
			else ()
				target_link_libraries (
					${SDL2W_TMP_TARGET}
					INTERFACE
						SDL2::SDL2
				)
			endif ()
		else ()
			add_library (${SDL2W_TMP_TARGET} STATIC IMPORTED)

			set_target_properties (
				${SDL2W_TMP_TARGET}
				PROPERTIES
					INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIRS}
			)

			if (MSVC)
				if (SDL2W_TMP_USE_STATIC AND SDL2W_TMP_SDL2_STATIC_LIB_LOCATION)
					set (SDL2W_TMP_LOCATION ${SDL2W_TMP_SDL2_STATIC_LIB_LOCATION})
				else ()
					set (SDL2W_TMP_LOCATION ${SDL2W_TMP_SDL2_LIB_LOCATION})
				endif ()

				set_target_properties (
					${SDL2W_TMP_TARGET}
					PROPERTIES
						IMPORTED_LOCATION ${SDL2W_TMP_LOCATION}
				)
			else ()
				set_target_properties (
					${SDL2W_TMP_TARGET}
					PROPERTIES
						IMPORTED_LOCATION ${SDL2_LIBRARIES}
				)
			endif ()
		endif ()

		if (WIN32 AND SDL2W_TMP_USE_STATIC)
			target_link_libraries (
				${SDL2W_TMP_TARGET}
				INTERFACE
					imm32
					setupapi
					version
					winmm
			)
		endif ()
	endif ()

	# Add main target.
	#
	if (NOT TARGET ${SDL2W_TMP_TARGET_MAIN})
		if (SDL2W_TMP_FOUND_TARGETS)
			add_library (${SDL2W_TMP_TARGET_MAIN} INTERFACE IMPORTED)

			if (MINGW)
				# Resolve undefined reference to WinMain.
				#
				target_link_libraries (
					${SDL2W_TMP_TARGET_MAIN}
					INTERFACE
						-lmingw32
				)
			endif ()

			target_link_libraries (
				${SDL2W_TMP_TARGET_MAIN}
				INTERFACE
					SDL2::SDL2main
			)
		else ()
			add_library (${SDL2W_TMP_TARGET_MAIN} STATIC IMPORTED)

			if (MINGW)
				# Resolve undefined reference to WinMain.
				#
				set_target_properties (
					${SDL2W_TMP_TARGET_MAIN}
					PROPERTIES
						IMPORTED_LINK_INTERFACE_LIBRARIES -lmingw32
				)
			endif ()

			set_target_properties (
				${SDL2W_TMP_TARGET_MAIN}
				PROPERTIES
					INTERFACE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIRS}
			)

			if (MSVC)
				set_target_properties (
					${SDL2W_TMP_TARGET_MAIN}
					PROPERTIES
						IMPORTED_LOCATION ${SDL2W_TMP_SDL2MAIN_LIB_LOCATION}
				)
			else ()
				set_target_properties (
					${SDL2W_TMP_TARGET_MAIN}
					PROPERTIES
						IMPORTED_LOCATION ${SDL2_LIBRARIES}
				)
			endif ()
		endif ()
	endif ()
else ()
	if (${CMAKE_FIND_PACKAGE_NAME}_FIND_REQUIRED)
		message (FATAL_ERROR "[SDL2W] SDL2 not found.")
	endif ()
endif ()
