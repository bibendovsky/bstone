project(bstone CXX)

cmake_minimum_required(VERSION 2.8)

find_package(SDL 2 REQUIRED)

option(BSTONE_PANDORA "Make Pandora build")


if (MSVC)
# Definitions
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_BIND_TO_CURRENT_VCLIBS_VERSION=1)

# Common compiler flags
set(TMP "${CMAKE_CXX_FLAGS}")
set(TMP "${TMP} /W4") # Warning Level
set(TMP "${TMP} /MP") # Multi-processor Compilation
set(TMP "${TMP} /GF") # Eliminate Duplicate Strings
set(TMP "${TMP} /Gm-") # Disable Minimal Rebuild
set(TMP "${TMP} /GR-") # Disable Run-Time Type Information
set(TMP "${TMP} /Y-") # Ignore Precompiled Header Options
set(CMAKE_CXX_FLAGS "${TMP}")

# Release compiler flags
set(TMP "${CMAKE_CXX_FLAGS_RELEASE}")
set(TMP "${TMP} /Oi") # Enable Intrinsic Function
set(TMP "${TMP} /Ot") # Favore size or speed
set(TMP "${TMP} /Oy") # Omit Frame Pointers
set(TMP "${TMP} /GS-") # Disable Security Check
set(TMP "${TMP} /Gy") # Enable Function-Level Linking
set(CMAKE_CXX_FLAGS_RELEASE "${TMP}")

# Release linker flags
set(TMP "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
set(TMP "${TMP} /OPT:REF") # References
set(TMP "${TMP} /OPT:ICF") # Enable COMDAT Folding
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${TMP}")
endif (MSVC)

if (MINGW)
# Common linker flags
set(TMP "${CMAKE_EXE_LINKER_FLAGS}")
set(TMP "${TMP} -static") # Link system libraries statically
set(CMAKE_EXE_LINKER_FLAGS "${TMP}")
endif (MINGW)


if (BSTONE_PANDORA)
# Definitions
add_definitions(-DBSTONE_PANDORA)
add_definitions(-DBSTONE_USE_GLES)
add_definitions(-DBSTONE_OGL_DIRECT_LINK)

# Common compiler flags
set(TMP "${CMAKE_CXX_FLAGS}")
set(TMP "${TMP} -fsigned-char")
set(TMP "${TMP} -mcpu=cortex-a8")
set(TMP "${TMP} -mfpu=neon")
set(TMP "${TMP} -mfloat-abi=softfp")
set(TMP "${TMP} -ffast-math")
set(TMP "${TMP} -fsingle-precision-constant")
set(TMP "${TMP} -fno-inline-functions")
set(CMAKE_CXX_FLAGS "${TMP}")

include_directories(
	$ENV{PNDSDK}
)
endif (BSTONE_PANDORA)


include_directories(
	${SDL_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/dosbox
)

set(hdrs
	${CMAKE_SOURCE_DIR}/dosbox/adlib.h
	${CMAKE_SOURCE_DIR}/dosbox/dbopl.h
	${CMAKE_SOURCE_DIR}/dosbox/dosbox.h
	${CMAKE_SOURCE_DIR}/dosbox/hardware.h
	${CMAKE_SOURCE_DIR}/dosbox/inout.h
	${CMAKE_SOURCE_DIR}/dosbox/mixer.h
	${CMAKE_SOURCE_DIR}/dosbox/pic.h
	${CMAKE_SOURCE_DIR}/dosbox/setup.h
	${CMAKE_SOURCE_DIR}/3d_def.h
	${CMAKE_SOURCE_DIR}/3d_menu.h
	${CMAKE_SOURCE_DIR}/an_codes.h
	${CMAKE_SOURCE_DIR}/audio.h
	${CMAKE_SOURCE_DIR}/audiobs1.h
	${CMAKE_SOURCE_DIR}/audiofir.h
	${CMAKE_SOURCE_DIR}/audiovsi.h
	${CMAKE_SOURCE_DIR}/bstone_adlib_decoder.h
	${CMAKE_SOURCE_DIR}/bstone_adlib_music_decoder.h
	${CMAKE_SOURCE_DIR}/bstone_adlib_sfx_decoder.h
	${CMAKE_SOURCE_DIR}/bstone_audio_decoder.h
	${CMAKE_SOURCE_DIR}/bstone_audio_mixer.h
	${CMAKE_SOURCE_DIR}/bstone_binary_reader.h
	${CMAKE_SOURCE_DIR}/bstone_binary_writer.h
	${CMAKE_SOURCE_DIR}/bstone_c.h
	${CMAKE_SOURCE_DIR}/bstone_cl_args.h
	${CMAKE_SOURCE_DIR}/bstone_endian.h
	${CMAKE_SOURCE_DIR}/bstone_file_stream.h
	${CMAKE_SOURCE_DIR}/bstone_format_string.h
	${CMAKE_SOURCE_DIR}/bstone_istream.h
	${CMAKE_SOURCE_DIR}/bstone_memory_binary_reader.h
	${CMAKE_SOURCE_DIR}/bstone_memory_stream.h
	${CMAKE_SOURCE_DIR}/bstone_ogl_api.h
	${CMAKE_SOURCE_DIR}/bstone_ogl_version.h
	${CMAKE_SOURCE_DIR}/bstone_opl2.h
	${CMAKE_SOURCE_DIR}/bstone_pcm_decoder.h
	${CMAKE_SOURCE_DIR}/bstone_string_helper.h
	${CMAKE_SOURCE_DIR}/bstone_un_value.h
	${CMAKE_SOURCE_DIR}/gfxe_bs1.h
	${CMAKE_SOURCE_DIR}/gfxe_fir.h
	${CMAKE_SOURCE_DIR}/gfxe_vsi.h
	${CMAKE_SOURCE_DIR}/gfxv_bs1.h
	${CMAKE_SOURCE_DIR}/gfxv_fir.h
	${CMAKE_SOURCE_DIR}/gfxv_vsi.h
	${CMAKE_SOURCE_DIR}/id_ca.h
	${CMAKE_SOURCE_DIR}/id_head.h
	${CMAKE_SOURCE_DIR}/id_heads.h
	${CMAKE_SOURCE_DIR}/id_in.h
	${CMAKE_SOURCE_DIR}/id_mm.h
	${CMAKE_SOURCE_DIR}/id_pm.h
	${CMAKE_SOURCE_DIR}/id_sd.h
	${CMAKE_SOURCE_DIR}/id_us.h
	${CMAKE_SOURCE_DIR}/id_vh.h
	${CMAKE_SOURCE_DIR}/id_vl.h
	${CMAKE_SOURCE_DIR}/jam_io.h
	${CMAKE_SOURCE_DIR}/jm_cio.h
	${CMAKE_SOURCE_DIR}/jm_debug.h
	${CMAKE_SOURCE_DIR}/jm_error.h
	${CMAKE_SOURCE_DIR}/jm_io.h
	${CMAKE_SOURCE_DIR}/jm_lzh.h
	${CMAKE_SOURCE_DIR}/jm_tp.h
	${CMAKE_SOURCE_DIR}/jm_vl.h
	${CMAKE_SOURCE_DIR}/mapsbs1.h
	${CMAKE_SOURCE_DIR}/mapsvsi.h
	${CMAKE_SOURCE_DIR}/movie.h
)

set(srcs
	${CMAKE_SOURCE_DIR}/dosbox/dbopl.cpp
	${CMAKE_SOURCE_DIR}/3d_act1.cpp
	${CMAKE_SOURCE_DIR}/3d_act2.cpp
	${CMAKE_SOURCE_DIR}/3d_agent.cpp
	${CMAKE_SOURCE_DIR}/3d_debug.cpp
	${CMAKE_SOURCE_DIR}/3d_draw.cpp
	${CMAKE_SOURCE_DIR}/3d_draw2.cpp
	${CMAKE_SOURCE_DIR}/3d_game.cpp
	${CMAKE_SOURCE_DIR}/3d_inter.cpp
	${CMAKE_SOURCE_DIR}/3d_main.cpp
	${CMAKE_SOURCE_DIR}/3d_menu.cpp
	${CMAKE_SOURCE_DIR}/3d_msgs.cpp
	${CMAKE_SOURCE_DIR}/3d_play.cpp
	${CMAKE_SOURCE_DIR}/3d_scale.cpp
	${CMAKE_SOURCE_DIR}/3d_state.cpp
	${CMAKE_SOURCE_DIR}/bstone_adlib_decoder.cpp
	${CMAKE_SOURCE_DIR}/bstone_adlib_music_decoder.cpp
	${CMAKE_SOURCE_DIR}/bstone_adlib_sfx_decoder.cpp
	${CMAKE_SOURCE_DIR}/bstone_audio_decoder.cpp
	${CMAKE_SOURCE_DIR}/bstone_audio_mixer.cpp
	${CMAKE_SOURCE_DIR}/bstone_binary_reader.cpp
	${CMAKE_SOURCE_DIR}/bstone_binary_writer.cpp
	${CMAKE_SOURCE_DIR}/bstone_c.cpp
	${CMAKE_SOURCE_DIR}/bstone_cl_args.cpp
	${CMAKE_SOURCE_DIR}/bstone_file_stream.cpp
	${CMAKE_SOURCE_DIR}/bstone_format_string.cpp
	${CMAKE_SOURCE_DIR}/bstone_istream.cpp
	${CMAKE_SOURCE_DIR}/bstone_memory_binary_reader.cpp
	${CMAKE_SOURCE_DIR}/bstone_memory_stream.cpp
	${CMAKE_SOURCE_DIR}/bstone_ogl_api.cpp
	${CMAKE_SOURCE_DIR}/bstone_ogl_version.cpp
	${CMAKE_SOURCE_DIR}/bstone_opl2.cpp
	${CMAKE_SOURCE_DIR}/bstone_pcm_decoder.cpp
	${CMAKE_SOURCE_DIR}/bstone_string_helper.cpp
	${CMAKE_SOURCE_DIR}/colormap.cpp
	${CMAKE_SOURCE_DIR}/d3_d2.cpp
	${CMAKE_SOURCE_DIR}/d3_dr2.cpp
	${CMAKE_SOURCE_DIR}/id_ca.cpp
	${CMAKE_SOURCE_DIR}/id_in.cpp
	${CMAKE_SOURCE_DIR}/id_mm.cpp
	${CMAKE_SOURCE_DIR}/id_pm.cpp
	${CMAKE_SOURCE_DIR}/id_sd.cpp
	${CMAKE_SOURCE_DIR}/id_sdd.cpp
	${CMAKE_SOURCE_DIR}/id_us.cpp
	${CMAKE_SOURCE_DIR}/id_us_1.cpp
	${CMAKE_SOURCE_DIR}/id_vh.cpp
	${CMAKE_SOURCE_DIR}/id_vl.cpp
	${CMAKE_SOURCE_DIR}/jm_cio.cpp
	${CMAKE_SOURCE_DIR}/jm_debug.cpp
	${CMAKE_SOURCE_DIR}/jm_free.cpp
	${CMAKE_SOURCE_DIR}/jm_io.cpp
	${CMAKE_SOURCE_DIR}/jm_lzh.cpp
	${CMAKE_SOURCE_DIR}/jm_tp.cpp
	${CMAKE_SOURCE_DIR}/markhack.cpp
	${CMAKE_SOURCE_DIR}/movie.cpp
	${CMAKE_SOURCE_DIR}/scale.cpp
	${CMAKE_SOURCE_DIR}/stub.cpp
	${CMAKE_SOURCE_DIR}/vgapal.cpp
)

add_executable(
	bstone
	${srcs}
	${hdrs}
)

if (MINGW)
target_link_libraries(bstone
	mingw32
)
endif (MINGW)

if (BSTONE_PANDORA)
target_link_libraries(bstone
	Xext
	Xau
	Xdmcp
	X11
	GLESv2
	EGL
	IMGegl
	srv_um
	ts
)
endif (BSTONE_PANDORA)

target_link_libraries(bstone
	${SDLMAIN_LIBRARY}
	${SDL_LIBRARY}
)
