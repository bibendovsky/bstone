# bstone
cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_flac VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::flac ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PUBLIC FLAC__NO_DLL)
target_compile_definitions(${PROJECT_NAME} PRIVATE
	FLAC__NO_ASM
	FLAC__HAS_OGG=0
	PACKAGE_VERSION=\"v1.5.0\"
)
target_compile_options(${PROJECT_NAME} PRIVATE $<$<C_COMPILER_ID:MSVC>:/W1>)
target_include_directories(${PROJECT_NAME} PUBLIC ../../include)
target_include_directories(${PROJECT_NAME} PRIVATE include)
# --------------------------------------
set(bstone_flac_include_flac_sources
	../../include/FLAC/assert.h
	../../include/FLAC/callback.h
	../../include/FLAC/export.h
	../../include/FLAC/format.h
	../../include/FLAC/ordinals.h
	../../include/FLAC/stream_decoder.h
)
source_group("include/FLAC" FILES ${bstone_flac_include_flac_sources})
target_sources(${PROJECT_NAME} PRIVATE ${bstone_flac_include_flac_sources})
# --------------------------------------
set(bstone_flac_include_share_sources
	../../include/share/alloc.h
	../../include/share/compat.h
	../../include/share/endswap.h
)
source_group("include/share" FILES ${bstone_flac_include_share_sources})
target_sources(${PROJECT_NAME} PRIVATE ${bstone_flac_include_share_sources})
# --------------------------------------
set(bstone_flac_include_private_sources
	include/private/bitmath.h
	include/private/bitreader.h
	include/private/cpu.h
	include/private/crc.h
	include/private/fixed.h
	include/private/float.h
	include/private/format.h
	include/private/lpc.h
	include/private/macros.h
	include/private/md5.h
	include/private/memory.h
)
source_group("include/private" FILES ${bstone_flac_include_private_sources})
target_sources(${PROJECT_NAME} PRIVATE ${bstone_flac_include_private_sources})
# --------------------------------------
set(bstone_flac_include_protected_sources
	include/protected/stream_decoder.h
)
source_group("include/protected" FILES ${bstone_flac_include_protected_sources})
target_sources(${PROJECT_NAME} PRIVATE ${bstone_flac_include_protected_sources})
# --------------------------------------
set(bstone_flac_include_src_sources
	src/bitmath.c
	src/bitreader.c
	src/cpu.c
	src/crc.c
	src/fixed.c
	src/format.c
	src/lpc.c
	src/md5.c
	src/memory.c
	src/stream_decoder.c
)
source_group("src" FILES ${bstone_flac_include_src_sources})
target_sources(${PROJECT_NAME} PRIVATE ${bstone_flac_include_src_sources})
