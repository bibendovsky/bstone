cmake_minimum_required(VERSION 3.1.3 FATAL_ERROR)
project(bstone VERSION 1.1.12 LANGUAGES CXX)


option(BSTONE_USE_STATIC_LINKING "Link build statically." OFF)
option(BSTONE_PANDORA "Build for Pandora." OFF)
option(BSTONE_USE_PCH "Use precompiled headers." OFF)
option(BSTONE_USE_MULTI_PROCESS_COMPILATION "Use multi-process compilation." ON)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(BSTONE_SDL2_COMPONENTS "")

if (BSTONE_USE_STATIC_LINKING)
	set(BSTONE_SDL2_COMPONENTS "${BSTONE_SDL2_COMPONENTS};static")
endif ()

find_package(SDL2W 2.0.4 REQUIRED COMPONENTS ${BSTONE_SDL2_COMPONENTS})


#
# Target.
#
add_executable(${PROJECT_NAME} "")


#
# Target properties.
#
set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
		CXX_STANDARD 14
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
)

if (WIN32)
	set_target_properties(
		${PROJECT_NAME}
		PROPERTIES
			WIN32_EXECUTABLE TRUE
	)
endif ()


#
# Target compile definitions.
#
if (BSTONE_USE_PCH)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE
			BSTONE_USE_PCH
	)
endif ()

if (MSVC)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE
			_CRT_SECURE_NO_WARNINGS
			_SCL_SECURE_NO_WARNINGS
			_BIND_TO_CURRENT_VCLIBS_VERSION=1
	)
endif ()

if (BSTONE_PANDORA)
	target_compile_definitions(
		${PROJECT_NAME}
		PRIVATE
			BSTONE_PANDORA
	)
endif ()


#
# Target compile options.
#
if (MSVC)
	if (BSTONE_USE_PCH)
		target_compile_options(
			${PROJECT_NAME}
			PRIVATE
				-Yubstone_precompiled.h
				-FIbstone_precompiled.h
		)
	endif ()

	if (BSTONE_USE_STATIC_LINKING)
		target_compile_options(
			${PROJECT_NAME}
			PRIVATE
				$<$<CONFIG:DEBUG>:-MTd>
				$<$<NOT:$<CONFIG:DEBUG>>:-MT>
		)
	endif ()

	if (BSTONE_USE_MULTI_PROCESS_COMPILATION)
		target_compile_options(
			${PROJECT_NAME}
			PRIVATE
				-MP
		)
	endif ()
endif ()

if (BSTONE_PANDORA)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			-fsigned-char
			-fsigned-char
			-mcpu=cortex-a8
			-mfpu=neon
			-mfloat-abi=softfp
			-ffast-math
			-fsingle-precision-constant
			-fno-inline-functions
	)
endif ()


#
# Target link libraries.
#
if (BSTONE_USE_STATIC_LINKING AND NOT WIN32)
	target_link_libraries(
		${PROJECT_NAME}
		-static
	)
endif ()

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		SDL2W
)

if (BSTONE_PANDORA)
	target_link_libraries(
		${PROJECT_NAME}
		PRIVATE
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
endif ()


#
# Target include directories.
#
target_include_directories(
	${PROJECT_NAME}
	PRIVATE
		dosbox
)

if (NOT WIN32 OR MINGW)
	target_include_directories(
		${PROJECT_NAME}
		PRIVATE
			dosbox
	)
endif ()

if (BSTONE_PANDORA)
	target_include_directories(
		${PROJECT_NAME}
		PRIVATE
			$ENV{PNDSDK}
	)
endif ()


#
# Target sources.
#
target_sources(
	${PROJECT_NAME}
	PRIVATE
		dosbox/adlib.h
		dosbox/dbopl.h
		dosbox/dosbox.h
		dosbox/hardware.h
		dosbox/inout.h
		dosbox/mixer.h
		dosbox/pic.h
		dosbox/setup.h
		3d_def.h
		3d_menu.h
		an_codes.h
		audio.h
		bstone_adlib_decoder.h
		bstone_adlib_music_decoder.h
		bstone_adlib_sfx_decoder.h
		bstone_audio_decoder.h
		bstone_audio_mixer.h
		bstone_archiver.h
		bstone_atomic.h
		bstone_binary_reader.h
		bstone_binary_writer.h
		bstone_cl_args.h
		bstone_crc32.h
		bstone_detail_ogl_renderer.h
		bstone_detail_ogl_1_x_renderer.h
		bstone_detail_ogl_renderer_utils.h
		bstone_detail_renderer_utils.h
		bstone_endian.h
		bstone_file_stream.h
		bstone_fizzle_fx.h
		bstone_fixed_point.h
		bstone_format_string.h
		bstone_generic_fizzle_fx.h
		bstone_stream.h
		bstone_log.h
		bstone_matn.h
		bstone_memory_binary_reader.h
		bstone_memory_stream.h
		bstone_missing_sprite_64x64_image.h
		bstone_missing_wall_64x64_image.h
		bstone_mt_queue_1r1w.h
		bstone_ogl.h
		bstone_opl2.h
		bstone_ref_values.h
		bstone_renderer.h
		bstone_renderer_utils.h
		bstone_pcm_decoder.h
		bstone_precompiled.h
		bstone_ps_fizzle_fx.h
		bstone_renderer_manager.h
		bstone_renderer_texture_manager.h
		bstone_scope_guard.h
		bstone_sha1.h
		bstone_sprite.h
		bstone_sprite_cache.h
		bstone_string_helper.h
		bstone_text_reader.h
		bstone_text_writer.h
		bstone_un_value.h
		bstone_vecn.h
		gfxv.h
		id_ca.h
		id_heads.h
		id_in.h
		id_pm.h
		id_sd.h
		id_us.h
		id_vh.h
		id_vl.h
		jam_io.h
		jm_cio.h
		jm_io.h
		jm_lzh.h
		jm_tp.h
		jm_vl.h
		movie.h
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		dosbox/dbopl.cpp
		3d_act1.cpp
		3d_act2.cpp
		3d_agent.cpp
		3d_debug.cpp
		3d_draw.cpp
		3d_draw2.cpp
		3d_game.cpp
		3d_inter.cpp
		3d_main.cpp
		3d_menu.cpp
		3d_msgs.cpp
		3d_play.cpp
		3d_scale.cpp
		3d_state.cpp
		bstone_adlib_decoder.cpp
		bstone_adlib_music_decoder.cpp
		bstone_adlib_sfx_decoder.cpp
		bstone_archiver_impl.cpp
		bstone_audio_decoder.cpp
		bstone_audio_mixer.cpp
		bstone_binary_reader.cpp
		bstone_binary_writer.cpp
		bstone_cl_args.cpp
		bstone_crc32.cpp
		bstone_detail_ogl_renderer.cpp
		bstone_detail_ogl_1_x_renderer.cpp
		bstone_detail_ogl_renderer_utils.cpp
		bstone_detail_renderer_utils.cpp
		bstone_file_stream.cpp
		bstone_fizzle_fx.cpp
		bstone_fixed_point.cpp
		bstone_format_string.cpp
		bstone_generic_fizzle_fx.cpp
		bstone_stream.cpp
		bstone_log.cpp
		bstone_memory_binary_reader.cpp
		bstone_memory_stream.cpp
		bstone_missing_sprite_64x64_image.cpp
		bstone_missing_wall_64x64_image.cpp
		bstone_ogl.cpp
		bstone_opl2.cpp
		bstone_pcm_decoder.cpp
		bstone_precompiled.cpp
		bstone_ps_fizzle_fx.cpp
		bstone_renderer_manager_impl.cpp
		bstone_renderer_texture_manager_impl.cpp
		bstone_renderer_utils.cpp
		bstone_sha1.cpp
		bstone_sprite.cpp
		bstone_sprite_cache.cpp
		bstone_string_helper.cpp
		bstone_text_reader.cpp
		bstone_text_writer.cpp
		colormap.cpp
		d3_d2.cpp
		d3_dr2.cpp
		id_ca.cpp
		id_in.cpp
		id_pm.cpp
		id_sd.cpp
		id_sdd.cpp
		id_us.cpp
		id_us_1.cpp
		id_vh.cpp
		id_vl.cpp
		jm_cio.cpp
		jm_free.cpp
		jm_io.cpp
		jm_lzh.cpp
		jm_tp.cpp
		markhack.cpp
		movie.cpp
		scale.cpp
		stub.cpp
		vgapal.cpp
)

if (WIN32)
	target_sources(
		${PROJECT_NAME}
		PRIVATE
			sys/win32/bstone_win32.rc
	)
endif ()


#
# Source properties.
#
if (MSVC AND BSTONE_USE_PCH)
	set_source_files_properties(
		bstone_precompiled.cpp
		PROPERTIES
			# Precompiled Header (Create)
			COMPILE_FLAGS -Yc
	)
endif ()


#
# Install.
#
install(
	TARGETS
		${PROJECT_NAME}
	RUNTIME DESTINATION "."
)

install(
	FILES
		../CHANGELOG.md
	DESTINATION "."
	RENAME bstone_changelog.txt
)

install(
	FILES
		../README.md
	DESTINATION "."
	RENAME bstone_readme.txt
)

install(
	FILES
		../LICENSE
	DESTINATION "."
	RENAME bstone_license.txt
)

install(
	FILES
		../COPYING-SDL2.txt
	DESTINATION "."
	RENAME bstone_sdl2_license.txt
)
