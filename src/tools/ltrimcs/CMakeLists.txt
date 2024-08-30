cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_ltrimcs VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[bstone_ltrimcs] v${PROJECT_VERSION}")

add_executable(bstone_ltrimcs src/bstone_ltrimcs.cpp)

set_target_properties(bstone_ltrimcs PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_options(bstone_ltrimcs
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)
