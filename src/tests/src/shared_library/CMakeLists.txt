#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
]]

cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)
project(bstone_tests_shared_library VERSION 0.0.1 LANGUAGES CXX)

add_library(${PROJECT_NAME} SHARED "")

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		OUTPUT_NAME "bstone_tests_shared_library"
		PREFIX ""
		SUFFIX ".bin"
)

target_include_directories(${PROJECT_NAME}
	PRIVATE
		../../..
		include
)

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

target_sources(${PROJECT_NAME}
	PRIVATE
		include/bstone_tests_shared_library.h
)

target_sources(${PROJECT_NAME}
	PRIVATE
		src/bstone_tests_shared_library.cpp
)

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "tests" RUNTIME DESTINATION "tests")
