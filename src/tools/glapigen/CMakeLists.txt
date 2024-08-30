cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_glapigen VERSION 1.0.0 LANGUAGES CXX)

message(STATUS "[bstone_glapigen] v${PROJECT_VERSION}")

add_executable(bstone_glapigen src/bstone_glapigen.cpp)

set_target_properties(bstone_glapigen PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

target_compile_definitions(bstone_glapigen
	PRIVATE
		"CMAKE_PROJECT_NAME=\"${PROJECT_NAME}\""
		"CMAKE_PROJECT_VERSION=\"${PROJECT_VERSION}\""
)

target_compile_options(bstone_glapigen
	PRIVATE
		$<$<CXX_COMPILER_ID:MSVC>:/W4>
		$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
)
