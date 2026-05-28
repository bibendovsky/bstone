cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_zlib VERSION 1.0.0 LANGUAGES C)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} STATIC)
add_library(bstone::lib::zlib ALIAS ${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_EXTENSIONS OFF
)
target_compile_definitions(${PROJECT_NAME} PUBLIC
	ZLIB_CONST
	Z_SOLO
)
target_include_directories(${PROJECT_NAME} PUBLIC include)
target_sources(${PROJECT_NAME} PRIVATE
	include/zconf.h
	include/zlib.h
	#
	src/adler32.c
	src/crc32.c
	src/crc32.h
	src/deflate.c
	src/deflate.h
	src/infback.c
	src/inffast.c
	src/inffast.h
	src/inffixed.h
	src/inflate.c
	src/inflate.h
	src/inftrees.c
	src/inftrees.h
	src/trees.c
	src/trees.h
	src/zutil.c
	src/zutil.h
)
