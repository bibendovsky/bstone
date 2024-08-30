cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_tests_process VERSION 1.0.0 LANGUAGES CXX)

add_executable(bstone_tests_process "")

set_target_properties(bstone_tests_process PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
	OUTPUT_NAME "bstone_tests_process"
	PREFIX ""
	SUFFIX ".bin"
)

target_compile_definitions(bstone_tests_process
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
)

target_compile_options(bstone_tests_process
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)

target_sources(bstone_tests_process PRIVATE src/bstone_tests_process.cpp)

install(TARGETS bstone_tests_process
	LIBRARY DESTINATION "bstone_tests"
	RUNTIME DESTINATION "bstone_tests"
)
