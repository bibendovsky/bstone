# bstone
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
# ======================================
project(bstone_lib_vorbis VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::vorbis ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_link_libraries(${PROJECT_NAME} PUBLIC bstone::lib::ogg)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE
	include/vorbis/codec.h
	include/vorbis/vorbisfile.h
	src/analysis.c
	src/backends.h
	src/bitrate.c
	src/bitrate.h
	src/block.c
	src/codebook.c
	src/codebook.h
	src/codec_internal.h
	src/envelope.c
	src/envelope.h
	src/floor0.c
	src/floor1.c
	src/highlevel.h
	src/info.c
	src/lookup.c
	src/lookup.h
	src/lookup_data.h
	src/lpc.c
	src/lpc.h
	src/lsp.c
	src/lsp.h
	src/mapping0.c
	src/masking.h
	src/mdct.c
	src/mdct.h
	src/misc.h
	src/os.h
	src/psy.c
	src/psy.h
	src/registry.c
	src/registry.h
	src/res0.c
	src/scales.h
	src/sharedbook.c
	src/smallft.c
	src/smallft.h
	src/synthesis.c
	src/window.c
	src/window.h
)
# ======================================
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_vorbisfile VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::vorbisfile ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_link_libraries(${PROJECT_NAME} PUBLIC bstone::lib::vorbis)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_sources(${PROJECT_NAME} PRIVATE src/vorbisfile.c)
