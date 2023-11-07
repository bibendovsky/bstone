#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
]]

cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
project(bstone_solution VERSION 1.0.2 LANGUAGES NONE)

message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")

option(BSTONE_TESTS "Build tests." OFF)
message(STATUS "[${PROJECT_NAME}] Build tests: ${BSTONE_TESTS}")

option(BSTONE_TRIM_FILE_PATHS_IN_EXE "Trim file names on the left in the executable." OFF)
message(STATUS "[${PROJECT_NAME}] Trim file names in the executable: ${BSTONE_TRIM_FILE_PATHS_IN_EXE}")

option(BSTONE_NO_ASSERT "Disable assertion." OFF)
message(STATUS "[${PROJECT_NAME}] Disable assertion: ${BSTONE_NO_ASSERT}")

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

if (BSTONE_TESTS)
	add_subdirectory(src/tests)
endif ()

add_subdirectory(src/tools/ltrimcs)
add_subdirectory (src)
