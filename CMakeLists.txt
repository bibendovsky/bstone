project(bstone CXX)
cmake_minimum_required(VERSION 2.8)


function (check_sdl_version TARGET_VERSION)
    if (NOT SDL2_INCLUDE_DIRS)
        set(BSTONE_MESSAGE
            ${BSTONE_MESSAGE}
            "\nSDL2 include directory not specified."
        )
    endif ()


    set(SDL2_LIBRARIES
        "SDL2_LIBRARIES-NOTFOUND"
        CACHE STRING
        "SDL2 libraries (use semicolon as separator).")

    if (NOT SDL2_LIBRARIES)
        set(BSTONE_MESSAGE
            ${BSTONE_MESSAGE}
            "\nSDL2 libraries not specified."
        )
    endif ()


    if (BSTONE_MESSAGE)
        message(FATAL_ERROR ${BSTONE_MESSAGE})
    endif ()

    unset(BSTONE_MESSAGE)


    file(
        STRINGS
        "${SDL2_INCLUDE_DIRS}/SDL_version.h"
        BSTONE_SDL2_VERSION_STRINGS
        REGEX "#define +SDL_(M(AJ|IN)OR_VERSION|PATCHLEVEL) +[0-9]+"
    )

    set(BSTONE_SDL2_MAJOR_VERSION "0")
    set(BSTONE_SDL2_MINOR_VERSION "0")
    set(BSTONE_SDL2_PATCH_VERSION "0")

    foreach (PIECE IN LISTS BSTONE_SDL2_VERSION_STRINGS)
        # Major

        set(PIECE_MATCH "")

        string(
            REGEX MATCH
            "MAJOR"
            PIECE_MATCH
            ${PIECE}
        )

        if (PIECE_MATCH)
            string(
                REGEX MATCH
                "[0-9]+"
                BSTONE_SDL2_MAJOR_VERSION
                ${PIECE}
            )
        endif ()


        # Minor

        set(PIECE_MATCH "")

        string(
            REGEX MATCH
            "MINOR"
            PIECE_MATCH
            ${PIECE}
        )

        if (PIECE_MATCH)
            string(
                REGEX MATCH
                "[0-9]+"
                BSTONE_SDL2_MINOR_VERSION
                ${PIECE}
            )
        endif ()


        # Patch

        set(PIECE_MATCH "")

        string(
            REGEX MATCH
            "PATCHLEVEL"
            PIECE_MATCH
            ${PIECE}
        )

        if (PIECE_MATCH)
            string(
                REGEX MATCH
                "[0-9]+"
                BSTONE_SDL2_PATCHLEVEL_VERSION
                ${PIECE}
            )
        endif ()
    endforeach (PIECE)

    set(BSTONE_SDL2_VERSION
        "${BSTONE_SDL2_MAJOR_VERSION}.${BSTONE_SDL2_MINOR_VERSION}.${BSTONE_SDL2_PATCHLEVEL_VERSION}"
    )

    message(STATUS "Found SDL ${BSTONE_SDL2_VERSION}.")

    if (BSTONE_SDL2_VERSION VERSION_LESS TARGET_VERSION)
        message(FATAL_ERROR "Expected SDL ${TARGET_VERSION}+")
    endif ()
endfunction (check_sdl_version)


if (MSVC)
    set(BSTONE_MESSAGE "")


    set(SDL2_INCLUDE_DIRS
        "SDL2_INCLUDE_DIRS-NOTFOUND"
        CACHE STRING
        "Path to SDL2 headers.")

    check_sdl_version("2.0.0")
else ()
    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
endif ()

option(BSTONE_USE_STATIC_CRT "Link CRT libraries statically.")
option(BSTONE_PANDORA "Build for Pandora.")
option(BSTONE_USE_PCH "Use precompiled headers.")

set(BSTONE_TARGET "bstone")
set(BSTONE_PCH_BASE_NAME "bstone_precompiled")

if (CMAKE_COMPILER_IS_GNUCXX)
    set(BSTONE_IS_GNU "TRUE")
endif ()

if (NOT MSVC)
    add_compile_options("-std=c++11")
endif ()

if (MSVC)
    add_definitions("-D_CRT_SECURE_NO_WARNINGS")
    add_definitions("-D_SCL_SECURE_NO_WARNINGS")
    add_definitions("-D_BIND_TO_CURRENT_VCLIBS_VERSION=1")

    if (BSTONE_USE_PCH)
        add_compile_options("-Yu${BSTONE_PCH_BASE_NAME}.h")
        add_compile_options("-FI${BSTONE_PCH_BASE_NAME}.h")
    endif ()

    if (BSTONE_USE_STATIC_CRT)
        add_compile_options("$<$<CONFIG:DEBUG>:-MTd>")
        add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:-MT>")
    endif ()
endif ()

if (MINGW)
    # Common linker flags
    set(TMP "${CMAKE_EXE_LINKER_FLAGS}")
    set(TMP "${TMP} -static") # Link system libraries statically
    set(CMAKE_EXE_LINKER_FLAGS "${TMP}")
endif ()

if (APPLE)
    add_definitions("-DBSTONE_OGL_DIRECT_LINK")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework OpenGL")
endif ()

if (BSTONE_PANDORA)
    # Definitions
    add_definitions("-DBSTONE_PANDORA")
    add_definitions("-DBSTONE_USE_GLES")
    add_definitions("-DBSTONE_OGL_DIRECT_LINK")

    # Common compiler flags
    add_compile_options("-fsigned-char")
    add_compile_options("-mcpu=cortex-a8")
    add_compile_options("-mfpu=neon")
    add_compile_options("-mfloat-abi=softfp")
    add_compile_options("-ffast-math")
    add_compile_options("-fsingle-precision-constant")
    add_compile_options("-fno-inline-functions")

    include_directories(
        $ENV{PNDSDK}
    )
endif ()

include_directories(
    "${SDL2_INCLUDE_DIRS}"
    "${CMAKE_SOURCE_DIR}/dosbox"
)

set(
    BSTONE_HEADERS
    "${CMAKE_SOURCE_DIR}/dosbox/adlib.h"
    "${CMAKE_SOURCE_DIR}/dosbox/dbopl.h"
    "${CMAKE_SOURCE_DIR}/dosbox/dosbox.h"
    "${CMAKE_SOURCE_DIR}/dosbox/hardware.h"
    "${CMAKE_SOURCE_DIR}/dosbox/inout.h"
    "${CMAKE_SOURCE_DIR}/dosbox/mixer.h"
    "${CMAKE_SOURCE_DIR}/dosbox/pic.h"
    "${CMAKE_SOURCE_DIR}/dosbox/setup.h"
    "${CMAKE_SOURCE_DIR}/3d_def.h"
    "${CMAKE_SOURCE_DIR}/3d_menu.h"
    "${CMAKE_SOURCE_DIR}/an_codes.h"
    "${CMAKE_SOURCE_DIR}/audio.h"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_decoder.h"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_music_decoder.h"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_sfx_decoder.h"
    "${CMAKE_SOURCE_DIR}/bstone_audio_decoder.h"
    "${CMAKE_SOURCE_DIR}/bstone_audio_mixer.h"
    "${CMAKE_SOURCE_DIR}/bstone_binary_reader.h"
    "${CMAKE_SOURCE_DIR}/bstone_binary_writer.h"
    "${CMAKE_SOURCE_DIR}/bstone_cl_args.h"
    "${CMAKE_SOURCE_DIR}/bstone_endian.h"
    "${CMAKE_SOURCE_DIR}/bstone_file_stream.h"
    "${CMAKE_SOURCE_DIR}/bstone_format_string.h"
    "${CMAKE_SOURCE_DIR}/bstone_istream.h"
    "${CMAKE_SOURCE_DIR}/bstone_log.h"
    "${CMAKE_SOURCE_DIR}/bstone_memory_binary_reader.h"
    "${CMAKE_SOURCE_DIR}/bstone_memory_stream.h"
    "${CMAKE_SOURCE_DIR}/bstone_ogl_api.h"
    "${CMAKE_SOURCE_DIR}/bstone_ogl_version.h"
    "${CMAKE_SOURCE_DIR}/bstone_opl2.h"
    "${CMAKE_SOURCE_DIR}/bstone_pcm_decoder.h"
    "${CMAKE_SOURCE_DIR}/bstone_string_helper.h"
    "${CMAKE_SOURCE_DIR}/bstone_un_value.h"
    "${CMAKE_SOURCE_DIR}/gfxv.h"
    "${CMAKE_SOURCE_DIR}/id_ca.h"
    "${CMAKE_SOURCE_DIR}/id_head.h"
    "${CMAKE_SOURCE_DIR}/id_heads.h"
    "${CMAKE_SOURCE_DIR}/id_in.h"
    "${CMAKE_SOURCE_DIR}/id_mm.h"
    "${CMAKE_SOURCE_DIR}/id_pm.h"
    "${CMAKE_SOURCE_DIR}/id_sd.h"
    "${CMAKE_SOURCE_DIR}/id_us.h"
    "${CMAKE_SOURCE_DIR}/id_vh.h"
    "${CMAKE_SOURCE_DIR}/id_vl.h"
    "${CMAKE_SOURCE_DIR}/jam_io.h"
    "${CMAKE_SOURCE_DIR}/jm_cio.h"
    "${CMAKE_SOURCE_DIR}/jm_debug.h"
    "${CMAKE_SOURCE_DIR}/jm_error.h"
    "${CMAKE_SOURCE_DIR}/jm_io.h"
    "${CMAKE_SOURCE_DIR}/jm_lzh.h"
    "${CMAKE_SOURCE_DIR}/jm_tp.h"
    "${CMAKE_SOURCE_DIR}/jm_vl.h"
    "${CMAKE_SOURCE_DIR}/movie.h"
    "${CMAKE_SOURCE_DIR}/${BSTONE_PCH_BASE_NAME}.h"
)

set(
    BSTONE_SOURCES
    "${CMAKE_SOURCE_DIR}/dosbox/dbopl.cpp"
    "${CMAKE_SOURCE_DIR}/3d_act1.cpp"
    "${CMAKE_SOURCE_DIR}/3d_act2.cpp"
    "${CMAKE_SOURCE_DIR}/3d_agent.cpp"
    "${CMAKE_SOURCE_DIR}/3d_debug.cpp"
    "${CMAKE_SOURCE_DIR}/3d_draw.cpp"
    "${CMAKE_SOURCE_DIR}/3d_draw2.cpp"
    "${CMAKE_SOURCE_DIR}/3d_game.cpp"
    "${CMAKE_SOURCE_DIR}/3d_inter.cpp"
    "${CMAKE_SOURCE_DIR}/3d_main.cpp"
    "${CMAKE_SOURCE_DIR}/3d_menu.cpp"
    "${CMAKE_SOURCE_DIR}/3d_msgs.cpp"
    "${CMAKE_SOURCE_DIR}/3d_play.cpp"
    "${CMAKE_SOURCE_DIR}/3d_scale.cpp"
    "${CMAKE_SOURCE_DIR}/3d_state.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_decoder.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_music_decoder.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_adlib_sfx_decoder.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_audio_decoder.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_audio_mixer.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_binary_reader.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_binary_writer.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_cl_args.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_file_stream.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_format_string.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_istream.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_log.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_memory_binary_reader.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_memory_stream.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_ogl_api.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_ogl_version.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_opl2.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_pcm_decoder.cpp"
    "${CMAKE_SOURCE_DIR}/bstone_string_helper.cpp"
    "${CMAKE_SOURCE_DIR}/colormap.cpp"
    "${CMAKE_SOURCE_DIR}/d3_d2.cpp"
    "${CMAKE_SOURCE_DIR}/d3_dr2.cpp"
    "${CMAKE_SOURCE_DIR}/id_ca.cpp"
    "${CMAKE_SOURCE_DIR}/id_in.cpp"
    "${CMAKE_SOURCE_DIR}/id_mm.cpp"
    "${CMAKE_SOURCE_DIR}/id_pm.cpp"
    "${CMAKE_SOURCE_DIR}/id_sd.cpp"
    "${CMAKE_SOURCE_DIR}/id_sdd.cpp"
    "${CMAKE_SOURCE_DIR}/id_us.cpp"
    "${CMAKE_SOURCE_DIR}/id_us_1.cpp"
    "${CMAKE_SOURCE_DIR}/id_vh.cpp"
    "${CMAKE_SOURCE_DIR}/id_vl.cpp"
    "${CMAKE_SOURCE_DIR}/jm_cio.cpp"
    "${CMAKE_SOURCE_DIR}/jm_debug.cpp"
    "${CMAKE_SOURCE_DIR}/jm_free.cpp"
    "${CMAKE_SOURCE_DIR}/jm_io.cpp"
    "${CMAKE_SOURCE_DIR}/jm_lzh.cpp"
    "${CMAKE_SOURCE_DIR}/jm_tp.cpp"
    "${CMAKE_SOURCE_DIR}/markhack.cpp"
    "${CMAKE_SOURCE_DIR}/movie.cpp"
    "${CMAKE_SOURCE_DIR}/scale.cpp"
    "${CMAKE_SOURCE_DIR}/${BSTONE_PCH_BASE_NAME}.cpp"
    "${CMAKE_SOURCE_DIR}/stub.cpp"
    "${CMAKE_SOURCE_DIR}/vgapal.cpp"
)

if (MSVC AND BSTONE_USE_PCH)
    set_source_files_properties(
        ${BSTONE_PCH_BASE_NAME}.cpp
        PROPERTIES
            # Precompiled Header (Create)
            COMPILE_FLAGS -Yc
    )
endif ()

add_executable(
    "${BSTONE_TARGET}"
    ${BSTONE_HEADERS}
    ${BSTONE_SOURCES}
)

if (MINGW)
    target_link_libraries(
        "${BSTONE_TARGET}"
        "mingw32"
    )
endif ()

if (BSTONE_PANDORA)
    target_link_libraries(
        "${BSTONE_TARGET}"
        "Xext"
        "Xau"
        "Xdmcp"
        "X11"
        "GLESv2"
        "EGL"
        "IMGegl"
        "srv_um"
        "ts"
    )
endif ()

target_link_libraries(
    "${BSTONE_TARGET}"
    "${SDLMAIN_LIBRARY}"
    "${SDL2_LIBRARIES}"
)

if (MINGW)
    target_link_libraries(
        "${BSTONE_TARGET}"
        "imm32"
        "version"
        "winmm"
    )
endif ()

if (WIN32)
    set_target_properties(
        "${BSTONE_TARGET}"
        PROPERTIES
        WIN32_EXECUTABLE "TRUE"
    )
endif ()

if (BSTONE_USE_STATIC_CRT)
    if (BSTONE_IS_GNU)
        set_target_properties(
            "${BSTONE_TARGET}"
            PROPERTIES
            LINK_FLAGS "-static"
        )
    endif ()
endif ()
