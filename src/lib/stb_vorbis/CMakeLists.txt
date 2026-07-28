# bstone
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
# ======================================
project(bstone_lib_stb_vorbis VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::stb_vorbis ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_include_directories(${PROJECT_NAME} PUBLIC .)
# stb_vorbis.c is the amalgamated source; only stb_vorbis_impl.c compiles it (the
# others are listed for IDE visibility but must not be compiled on their own).
set_source_files_properties(stb_vorbis.c stb_vorbis.h PROPERTIES HEADER_FILE_ONLY TRUE)
target_sources(${PROJECT_NAME} PRIVATE
	stb_vorbis_impl.c
	stb_vorbis.c
	stb_vorbis.h
)
