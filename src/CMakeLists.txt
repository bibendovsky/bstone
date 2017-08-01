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

    check_sdl_version("2.0.3")
else ()
    INCLUDE(FindPkgConfig)
    PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2>=2.0.1)
endif ()

option(BSTONE_USE_STATIC_LINKING "Link build statically.")
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

    if (BSTONE_USE_STATIC_LINKING)
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

if (BSTONE_PANDORA)
    # Definitions
    add_definitions("-DBSTONE_PANDORA")

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
    "dosbox"
)

set(
    BSTONE_HEADERS
    "dosbox/adlib.h"
    "dosbox/dbopl.h"
    "dosbox/dosbox.h"
    "dosbox/hardware.h"
    "dosbox/inout.h"
    "dosbox/mixer.h"
    "dosbox/pic.h"
    "dosbox/setup.h"
    "3d_def.h"
    "3d_menu.h"
    "an_codes.h"
    "audio.h"
    "bstone_adlib_decoder.h"
    "bstone_adlib_music_decoder.h"
    "bstone_adlib_sfx_decoder.h"
    "bstone_audio_decoder.h"
    "bstone_audio_mixer.h"
    "bstone_atomic.h"
    "bstone_binary_reader.h"
    "bstone_binary_writer.h"
    "bstone_cl_args.h"
    "bstone_crc32.h"
    "bstone_endian.h"
    "bstone_file_stream.h"
    "bstone_fizzle_fx.h"
    "bstone_fixed_point.h"
    "bstone_format_string.h"
    "bstone_generic_fizzle_fx.h"
    "bstone_istream.h"
    "bstone_log.h"
    "bstone_memory_binary_reader.h"
    "bstone_memory_stream.h"
    "bstone_mt_queue_1r1w.h"
    "bstone_opl2.h"
    "bstone_ref_values.h"
    "bstone_pcm_decoder.h"
    "bstone_ps_fizzle_fx.h"
    "bstone_sha1.h"
    "bstone_sprite.h"
    "bstone_sprite_cache.h"
    "bstone_string_helper.h"
    "bstone_text_reader.h"
    "bstone_text_writer.h"
    "bstone_un_value.h"
    "gfxv.h"
    "id_ca.h"
    "id_heads.h"
    "id_in.h"
    "id_pm.h"
    "id_sd.h"
    "id_us.h"
    "id_vh.h"
    "id_vl.h"
    "jam_io.h"
    "jm_cio.h"
    "jm_io.h"
    "jm_lzh.h"
    "jm_tp.h"
    "jm_vl.h"
    "movie.h"
    "${BSTONE_PCH_BASE_NAME}.h"
)

set(
    BSTONE_SOURCES
    "dosbox/dbopl.cpp"
    "3d_act1.cpp"
    "3d_act2.cpp"
    "3d_agent.cpp"
    "3d_debug.cpp"
    "3d_draw.cpp"
    "3d_draw2.cpp"
    "3d_game.cpp"
    "3d_inter.cpp"
    "3d_main.cpp"
    "3d_menu.cpp"
    "3d_msgs.cpp"
    "3d_play.cpp"
    "3d_scale.cpp"
    "3d_state.cpp"
    "bstone_adlib_decoder.cpp"
    "bstone_adlib_music_decoder.cpp"
    "bstone_adlib_sfx_decoder.cpp"
    "bstone_audio_decoder.cpp"
    "bstone_audio_mixer.cpp"
    "bstone_binary_reader.cpp"
    "bstone_binary_writer.cpp"
    "bstone_cl_args.cpp"
    "bstone_crc32.cpp"
    "bstone_file_stream.cpp"
    "bstone_fizzle_fx.cpp"
    "bstone_format_string.cpp"
    "bstone_generic_fizzle_fx.cpp"
    "bstone_istream.cpp"
    "bstone_log.cpp"
    "bstone_memory_binary_reader.cpp"
    "bstone_memory_stream.cpp"
    "bstone_opl2.cpp"
    "bstone_pcm_decoder.cpp"
    "bstone_ps_fizzle_fx.cpp"
    "bstone_sha1.cpp"
    "bstone_sprite.cpp"
    "bstone_sprite_cache.cpp"
    "bstone_string_helper.cpp"
    "bstone_text_reader.cpp"
    "bstone_text_writer.cpp"
    "colormap.cpp"
    "d3_d2.cpp"
    "d3_dr2.cpp"
    "id_ca.cpp"
    "id_in.cpp"
    "id_pm.cpp"
    "id_sd.cpp"
    "id_sdd.cpp"
    "id_us.cpp"
    "id_us_1.cpp"
    "id_vh.cpp"
    "id_vl.cpp"
    "jm_cio.cpp"
    "jm_free.cpp"
    "jm_io.cpp"
    "jm_lzh.cpp"
    "jm_tp.cpp"
    "markhack.cpp"
    "movie.cpp"
    "scale.cpp"
    "${BSTONE_PCH_BASE_NAME}.cpp"
    "stub.cpp"
    "vgapal.cpp"
)

if (WIN32)
    set(
        BSTONE_SOURCES
        ${BSTONE_SOURCES}
        "sys/win32/bstone_win32.rc"
    )
endif ()

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
    "${SDL2_LIBRARIES}"
)

if (BSTONE_IS_GNU)
    target_link_libraries(
        "${BSTONE_TARGET}"
        "pthread"
    )
endif ()

if (MINGW OR (MSVC AND BSTONE_USE_STATIC_LINKING))
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

if (BSTONE_USE_STATIC_LINKING)
    if (BSTONE_IS_GNU)
        set_target_properties(
            "${BSTONE_TARGET}"
            PROPERTIES
            LINK_FLAGS "-static"
        )
    endif ()
endif ()


install(
    TARGETS
    "${BSTONE_TARGET}"
    RUNTIME DESTINATION "."
)

install(
    FILES
    "../README.md"
    DESTINATION "."
    RENAME "bstone_readme.txt"
)

install(
    FILES
    "../LICENSE"
    DESTINATION "."
    RENAME "bstone_license.txt"
)

install(
    FILES
    "../COPYING-SDL2.txt"
    DESTINATION "."
    RENAME "bstone_sdl2_license.txt"
)
