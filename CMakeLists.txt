project(bstone CXX)

cmake_minimum_required(VERSION 2.8)

find_package(SDL)

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

include_directories(
	${SDL_INCLUDE_DIR}
	${CMAKE_SOURCE_DIR}/dosbox
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
	${CMAKE_SOURCE_DIR}/bstone_file_stream.cpp
	${CMAKE_SOURCE_DIR}/bstone_format_string.cpp
	${CMAKE_SOURCE_DIR}/bstone_istream.cpp
	${CMAKE_SOURCE_DIR}/bstone_memory_binary_reader.cpp
	${CMAKE_SOURCE_DIR}/bstone_memory_stream.cpp
	${CMAKE_SOURCE_DIR}/bstone_ogl_api.cpp
	${CMAKE_SOURCE_DIR}/bstone_ogl_version.cpp
	${CMAKE_SOURCE_DIR}/bstone_opl2.cpp
	${CMAKE_SOURCE_DIR}/bstone_pcm_decoder.cpp
	${CMAKE_SOURCE_DIR}/colormap.cpp
	${CMAKE_SOURCE_DIR}/d3.cpp
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
)

target_link_libraries(bstone
	${SDL_LIBRARY}
	${SDLMAIN_LIBRARY}
)
