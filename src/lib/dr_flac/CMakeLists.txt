# bstone
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
# ======================================
project(bstone_lib_dr_flac VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::dr_flac ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_include_directories(${PROJECT_NAME} PUBLIC .)
# Only dr_flac_impl.c defines DR_FLAC_IMPLEMENTATION; dr_flac.h is listed for IDE
# visibility but is a plain header everywhere else.
set_source_files_properties(dr_flac.h PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(${PROJECT_NAME} PRIVATE
	dr_flac_impl.c
	dr_flac.h
)
