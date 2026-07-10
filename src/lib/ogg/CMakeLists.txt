# bstone
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_ogg VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::ogg ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE
	include/ogg/config_types.h
	include/ogg/ogg.h
	include/ogg/os_types.h
	src/bitwise.c
	src/crctable.h
	src/framing.c
)
