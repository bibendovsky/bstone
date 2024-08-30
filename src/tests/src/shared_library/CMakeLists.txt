cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_tests_shared_library VERSION 1.0.0 LANGUAGES CXX)

add_library(bstone_tests_shared_library SHARED "")

set_target_properties(bstone_tests_shared_library PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	OUTPUT_NAME "bstone_tests_shared_library"
	PREFIX ""
	SUFFIX ".bin"
)

target_include_directories(bstone_tests_shared_library
	PRIVATE
		../../../bstone/src
		include
)

target_compile_definitions(bstone_tests_shared_library
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
)

target_compile_options(bstone_tests_shared_library
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

target_sources(bstone_tests_shared_library
	PRIVATE
		include/bstone_tests_shared_library.h
)

target_sources(bstone_tests_shared_library
	PRIVATE
		src/bstone_tests_shared_library.cpp
)

install(TARGETS bstone_tests_shared_library
	LIBRARY DESTINATION "bstone_tests"
	RUNTIME DESTINATION "bstone_tests"
)
