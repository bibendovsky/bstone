#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
]]

cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
project(bstone_tests VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")

add_executable(${PROJECT_NAME} "")

target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
)

target_compile_options(${PROJECT_NAME}
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		${PROJECT_SOURCE_DIR}/..
)

target_sources(${PROJECT_NAME}
	PRIVATE
		../bstone_char_traits.cpp
		../bstone_char_traits.h
		../bstone_exception.cpp
		../bstone_exception.h
		../bstone_int.cpp
		../bstone_int.h
		../bstone_source_location.cpp
		../bstone_source_location.h
		../bstone_utility.cpp
		../bstone_utility.h
	PRIVATE
		src/bstone_tests_char_traits.cpp
		src/bstone_tests_exception.cpp
		src/bstone_tests_int.cpp
		src/bstone_tests_source_location.cpp
		src/bstone_tests_utility.cpp
	PRIVATE
		src/bstone_tester.cpp
		src/bstone_tester.h
		src/bstone_tests.cpp
)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ".")
