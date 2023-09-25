#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
]]


cmake_minimum_required(VERSION 3.4.0 FATAL_ERROR)
project(bstone VERSION 1.2.13 LANGUAGES CXX)


#
# Version.
#
set(BSTONE_VERSION "${PROJECT_VERSION}")
set(BSTONE_VERSION_PRE_RELEASE "wip")

if (NOT ("${BSTONE_VERSION_PRE_RELEASE}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}-${BSTONE_VERSION_PRE_RELEASE}")
endif ()

if (NOT ("${BSTONE_VERSION_BUILD}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}+${BSTONE_VERSION_BUILD}")
endif ()

message(STATUS "[${PROJECT_NAME}] v${BSTONE_VERSION}")


#
# Options.
#
option(BSTONE_USE_STATIC_LINKING "Link build statically." OFF)
option(BSTONE_PANDORA "Build for Pandora." OFF)
option(BSTONE_USE_PCH "Use precompiled headers if available." OFF)
option(BSTONE_USE_MULTI_PROCESS_COMPILATION "Use multi-process compilation." ON)
option(BSTONE_MORE_COMPILER_WARNINGS "Enables more compiler warnings." OFF)


# SDL2W
#
set(BSTONE_SDL_COMPONENTS "")

if (BSTONE_USE_STATIC_LINKING)
	set(BSTONE_SDL_COMPONENTS "${BSTONE_SDL_COMPONENTS};static")
endif ()

find_package(SDL2W 2.0.4 REQUIRED COMPONENTS ${BSTONE_SDL_COMPONENTS})

# Threads
#
set(THREADS_PREFER_PTHREAD_FLAG "TRUE")

find_package(Threads REQUIRED)


#
# Constants.
#
if (MSVC)
	set (BSTONE_MSVC "1")
else ()
	set (BSTONE_MSVC "0")
endif ()

#
# Target.
#
add_executable(${PROJECT_NAME} "")

add_custom_command(
	TARGET ${PROJECT_NAME}
	POST_BUILD
	COMMAND $<$<CONFIG:RELEASE>:${CMAKE_STRIP}> $<$<CONFIG:RELEASE>:$<TARGET_FILE:${PROJECT_NAME}>>
	VERBATIM
)

if (BSTONE_TRIM_FILE_PATHS_IN_EXE)
	add_dependencies(${PROJECT_NAME} bstone_ltrimcs)

	add_custom_command(
		TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND
			$<TARGET_FILE:bstone_ltrimcs>
				$<SHELL_PATH:$<TARGET_FILE:${PROJECT_NAME}>>
				$<SHELL_PATH:${PROJECT_SOURCE_DIR}/>
		VERBATIM
	)
endif ()

#
# Configure files.
#
configure_file(
	"${PROJECT_SOURCE_DIR}/bstone_version_.h.in"
	"${PROJECT_BINARY_DIR}/bstone_version_.h"
)

if (WIN32)
	configure_file(
		"${PROJECT_SOURCE_DIR}/sys/win32/bstone_win32.rc.in"
		"${PROJECT_BINARY_DIR}/sys/win32/bstone_win32.rc"
	)
endif ()


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
target_compile_definitions(
	${PROJECT_NAME}
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
		$<$<BOOL:${WIN32}>:_SCL_SECURE_NO_WARNINGS>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
)

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
if (BSTONE_MORE_COMPILER_WARNINGS)
	target_compile_options(
		${PROJECT_NAME}
		PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/W4>
			$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
	)
endif ()

if (MSVC)
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
# Target link options.
#
if (MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -DEBUG:FULL")
endif ()


#
# Target link libraries.
#
if (BSTONE_USE_STATIC_LINKING)
	if (WIN32)
		target_link_libraries(
			${PROJECT_NAME}
			PRIVATE
				$<$<CXX_COMPILER_ID:Clang>:-static>
				$<$<CXX_COMPILER_ID:GNU>:-static>
		)
	else ()
		target_link_libraries(
			${PROJECT_NAME}
			PRIVATE
				$<$<CXX_COMPILER_ID:Clang>:-static>
				$<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
				$<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
		)
	endif ()
endif ()

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		SDL2W::SDL2W
		Threads::Threads
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
		.
		dosbox
		lib/openal_soft/include/AL
		lib/stb
		lib/xbrz
		sys/win32
		${PROJECT_BINARY_DIR}
)

if (BSTONE_PANDORA)
	target_include_directories(
		${PROJECT_NAME}
		PRIVATE
			$ENV{PNDSDK}
	)
endif ()


if (BSTONE_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16.0"))
	target_precompile_headers(
		${PROJECT_NAME}
		PRIVATE
			bstone_precompiled.h
	)
endif ()

#
# Target sources.
#
target_sources(
	${PROJECT_NAME}
	PRIVATE
		bstone_int.h
		bstone_int.cpp
		bstone_char_traits.cpp
		bstone_char_traits.h
		bstone_utility.cpp
		bstone_utility.h
		bstone_source_location.cpp
		bstone_source_location.h
		bstone_span.cpp
		bstone_span.h
		bstone_algorithm.cpp
		bstone_algorithm.h
		bstone_ascii.cpp
		bstone_ascii.h
		bstone_endian.cpp
		bstone_endian.h
		bstone_memory.cpp
		bstone_memory.h
		bstone_scope_exit.cpp
		bstone_scope_exit.h
		bstone_array.cpp
		bstone_array.h
		bstone_unique_resource.cpp
		bstone_unique_resource.h
		bstone_cgm_mat.cpp
		bstone_cgm_mat.h
		bstone_cgm_clip_space.cpp
		bstone_cgm_clip_space.h
		bstone_cgm_transform.cpp
		bstone_cgm_transform.h
		bstone_cgm_vec.cpp
		bstone_cgm_vec.h
		bstone_unicode.cpp
		bstone_unicode.h
		bstone_utf.cpp
		bstone_utf.h
		bstone_utf8.cpp
		bstone_utf8.h
		bstone_utf16.cpp
		bstone_utf16.h
		bstone_single_pool_memory_resource.cpp
		bstone_single_pool_memory_resource.h
		bstone_fixed_pool_memory_resource.cpp
		bstone_fixed_pool_memory_resource.h
		bstone_generic_pool_memory_resource.cpp
		bstone_generic_pool_memory_resource.h
		bstone_configurations.h
		bstone_auto_arena_memory_resource.cpp
		bstone_auto_arena_memory_resource.h
		bstone_ts_auto_arena_memory_resource.cpp
		bstone_ts_auto_arena_memory_resource.h
		bstone_win32_wstring.cpp
		bstone_win32_wstring.h
)

target_sources(
	${PROJECT_NAME}
	PRIVATE
		dosbox/adlib.h
		dosbox/dbopl.h
		dosbox/dosbox.h
		dosbox/hardware.h
		dosbox/inout.h
		dosbox/mixer.cpp
		dosbox/mixer.h
		dosbox/pic.h
		dosbox/setup.h

		bstone_extent_2d.h
		bstone_offset_2d.h

		bstone_r2_rect.h
		bstone_r2_extent.h
		bstone_r2_offset.h

		bstone_hw_shader_registry.h
		bstone_hw_texture_mgr.h
		bstone_missing_sprite_64x64_image.h
		bstone_missing_wall_64x64_image.h
		bstone_renderer_type.h

		bstone_r3r.h
		bstone_r3r_buffer.h
		bstone_r3r_cmd_buffer.h
		bstone_r3r_limits.h
		bstone_r3r_mgr.h
		bstone_r3r_r2_texture.h
		bstone_r3r_sampler.h
		bstone_r3r_shader.h
		bstone_r3r_shader_stage.h
		bstone_r3r_shader_var.h
		bstone_r3r_tests.h
		bstone_r3r_types.h
		bstone_r3r_utils.h
		bstone_r3r_vertex_input.h

		bstone_gl_r3r.h
		bstone_gl_r3r_api.h
		bstone_gl_r3r_buffer.h
		bstone_gl_r3r_context.h
		bstone_gl_r3r_device_features.h
		bstone_gl_r3r_error.h
		bstone_gl_r3r_extension_mgr.h
		bstone_gl_r3r_utils.h
		bstone_gl_r3r_sampler.h
		bstone_gl_r3r_sampler_mgr.h
		bstone_gl_r3r_shader.h
		bstone_gl_r3r_shader_stage.h
		bstone_gl_r3r_shader_var.h
		bstone_gl_r3r_r2_texture.h
		bstone_gl_r3r_version.h
		bstone_gl_r3r_vertex_input.h
		bstone_gl_r3r_vertex_input_mgr.h

		3d_def.h
		3d_menu.h
		an_codes.h
		audio.h
		gfxv.h
		id_ca.h
		id_heads.h
		id_in.h
		id_sd.h
		id_us.h
		id_vh.h
		id_vl.h
		jm_cio.h
		jm_lzh.h
		jm_tp.h
		jm_vl.h
		movie.h

		bstone_adlib_decoder.h
		bstone_adlib_music_decoder.h
		bstone_adlib_sfx_decoder.h
		bstone_memory_pool_bitmap.cpp
		bstone_memory_pool_bitmap.h
		bstone_archiver.h
		bstone_atomic_flag.h
		bstone_audio_content_mgr.h
		bstone_audio_decoder.h
		bstone_audio_extractor.h
		bstone_audio_mixer.h
		bstone_audio_mixer_output_gains.h
		bstone_audio_mixer_utils.h
		bstone_audio_mixer_validator.h
		bstone_audio_mixer_voice_handle.h
		bstone_audio_mixer_voice_handle_mgr.h
		bstone_audio_sample_converter.h
		bstone_binary_reader.h
		bstone_binary_writer.h
		bstone_bmp_image_common.h
		bstone_bmp_image_decoder.h
		bstone_char_conv.h
		bstone_cl.h
		bstone_ccmd.h
		bstone_ccmd_action.h
		bstone_ccmd_mgr.h
		bstone_char_hasher.cpp
		bstone_char_hasher.h
		bstone_content_path.h
		bstone_crc32.h
		bstone_cvalidator.h
		bstone_cvar.h
		bstone_cvar_mgr.h
		bstone_cvar_string.h
		bstone_door.h
		bstone_dosbox_dbopl.h
		bstone_shared_library.h
		bstone_encoding.h
		bstone_enum_flags.h
		bstone_entry_point.h
		bstone_exception.h
		bstone_exception_utils.h
		bstone_file_stream.h
		bstone_file_system.h
		bstone_fizzle_fx.h
		bstone_format_string.h
		bstone_game_ticker.h
		bstone_generic_fizzle_fx.h
		bstone_globals.h
		bstone_hw_video.h
		bstone_image_decoder.h
		bstone_image_encoder.h
		bstone_image_extractor.h
		bstone_logger.h
		bstone_math.h
		bstone_memory_binary_reader.h
		bstone_memory_resource.h
		bstone_memory_stream.h
		bstone_mod_value.h
		bstone_mt_task_mgr.h
		bstone_nuked_opl3.h
		bstone_oal_audio_mixer.h
		bstone_oal_loader.h
		bstone_oal_resource.h
		bstone_oal_source.h
		bstone_oal_symbols.h
		bstone_opl3.h
		bstone_page_mgr.h
		bstone_pc_speaker_audio_decoder.h
		bstone_pcm_audio_decoder.h
		bstone_platform.h
		bstone_precompiled.h
		bstone_ps_fizzle_fx.h
		bstone_ref_values.h
		bstone_rgb8.h
		bstone_rgb_palette.h
		bstone_sha1.h
		bstone_sprite.h
		bstone_sprite_cache.h
		bstone_stb_image_encoder.h
		bstone_stb_image_utils.h
		bstone_stream.h
		bstone_string_helper.h
		bstone_string_view.h
		bstone_string_view_hasher.h
		bstone_sw_video.h
		bstone_system_audio_mixer.h
		bstone_text_extractor.h
		bstone_text_reader.h
		bstone_text_writer.h
		bstone_time.h
		bstone_type_traits.h
		bstone_version.h
		bstone_video.h
		bstone_voice.h
		bstone_voice_group.h
		bstone_zstring_view.h
		bstone_zstring_view_hasher.h

		bstone_sys_audio_mgr.h
		bstone_sys_color.h
		bstone_sys_push_audio_device.h
		bstone_sys_event.h
		bstone_sys_event_mgr.h
		bstone_sys_gl_context.h
		bstone_sys_gl_context_attributes.h
		bstone_sys_gl_context_profile.h
		bstone_sys_gl_mgr.h
		bstone_sys_gl_shared_library.h
		bstone_sys_logger.h
		bstone_sys_message_box.h
		bstone_sys_mouse.h
		bstone_sys_mouse_mgr.h
		bstone_sys_pixel_format.h
		bstone_sys_renderer.h
		bstone_sys_system_mgr.h
		bstone_sys_texture_lock.h
		bstone_sys_texture.h
		bstone_sys_video_mgr.h
		bstone_sys_virtual_key.h
		bstone_sys_window.h
		bstone_sys_window_mgr.h

		bstone_sys_sdl_audio_mgr.h
		bstone_sys_sdl_detail.h
		bstone_sys_sdl_limits.h
		bstone_sys_sdl_push_audio_device.h
		bstone_sys_sdl_event_mgr.h
		bstone_sys_sdl_exception.h
		bstone_sys_sdl_gl_context.h
		bstone_sys_sdl_gl_mgr.h
		bstone_sys_sdl_gl_shared_library.h
		bstone_sys_sdl_mouse_mgr.h
		bstone_sys_sdl_renderer.h
		bstone_sys_sdl_texture.h
		bstone_sys_sdl_texture_lock.h
		bstone_sys_sdl_video_mgr.h
		bstone_sys_sdl_window.h
		bstone_sys_sdl_window_mgr.h

		bstone_configurations.cpp
)

if (WIN32)
	target_sources(
		${PROJECT_NAME}
		PRIVATE
			sys/win32/bstone_win32_rc.h
			sys/win32/bstone_win32_registry.h
			$<$<CXX_COMPILER_ID:MSVC>:sys/win32/bstone_win32_msvc.manifest>
	)
endif ()

target_sources(
	${PROJECT_NAME}
	PRIVATE
		dosbox/dbopl.cpp

		lib/xbrz/xbrz.cpp

		bstone_extent_2d.cpp
		bstone_offset_2d.cpp
		bstone_rect_2d.cpp

		bstone_hw_shader_registry.cpp
		bstone_hw_texture_mgr.cpp
		bstone_missing_sprite_64x64_image.cpp
		bstone_missing_wall_64x64_image.cpp

		bstone_r3r.cpp
		bstone_r3r_buffer.cpp
		bstone_r3r_cmd_buffer.cpp
		bstone_r3r_mgr.cpp
		bstone_r3r_r2_texture.cpp
		bstone_r3r_sampler.cpp
		bstone_r3r_shader.cpp
		bstone_r3r_shader_stage.cpp
		bstone_r3r_shader_var.cpp
		bstone_r3r_utils.cpp

		bstone_gl_r3r.cpp
		bstone_gl_r3r_api.cpp
		bstone_gl_r3r_buffer.cpp
		bstone_gl_r3r_context.cpp
		bstone_gl_r3r_error.cpp
		bstone_gl_r3r_extension_mgr.cpp
		bstone_gl_r3r_utils.cpp
		bstone_gl_r3r_sampler.cpp
		bstone_gl_r3r_sampler_mgr.cpp
		bstone_gl_r3r_shader.cpp
		bstone_gl_r3r_shader_stage.cpp
		bstone_gl_r3r_shader_var.cpp
		bstone_gl_r3r_r2_texture.cpp
		bstone_gl_r3r_vertex_input.cpp
		bstone_gl_r3r_vertex_input_mgr.cpp

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
		colormap.cpp
		d3_d2.cpp
		d3_dr2.cpp
		id_ca.cpp
		id_in.cpp
		id_sd.cpp
		id_us.cpp
		id_us_1.cpp
		id_vh.cpp
		id_vl.cpp
		jm_free.cpp
		jm_lzh.cpp
		jm_tp.cpp
		markhack.cpp
		movie.cpp
		scale.cpp
		stub.cpp
		vgapal.cpp

		bstone_adlib_decoder.cpp
		bstone_adlib_music_decoder.cpp
		bstone_adlib_sfx_decoder.cpp
		bstone_archiver.cpp
		bstone_atomic_flag.cpp
		bstone_audio_content_mgr.cpp
		bstone_audio_decoder.cpp
		bstone_audio_extractor.cpp
		bstone_audio_mixer.cpp
		bstone_audio_mixer_utils.cpp
		bstone_audio_mixer_validator.cpp
		bstone_audio_mixer_voice_handle.cpp
		bstone_audio_mixer_voice_handle_mgr.cpp
		bstone_audio_sample_converter.cpp
		bstone_binary_reader.cpp
		bstone_binary_writer.cpp
		bstone_bmp_image_decoder.cpp
		bstone_ccmd.cpp
		bstone_ccmd_action.cpp
		bstone_ccmd_mgr.cpp
		bstone_cvalidator.cpp
		bstone_char_conv.cpp
		bstone_cl.cpp
		bstone_content_path.cpp
		bstone_crc32.cpp
		bstone_cvar.cpp
		bstone_cvar_mgr.cpp
		bstone_cvar_string.cpp
		bstone_dosbox_dbopl.cpp
		bstone_encoding.cpp
		bstone_entry_point.cpp
		bstone_exception.cpp
		bstone_exception_utils.cpp
		bstone_file_stream.cpp
		bstone_file_system.cpp
		bstone_fizzle_fx.cpp
		bstone_format_string.cpp
		bstone_game_ticker.cpp
		bstone_generic_fizzle_fx.cpp
		bstone_globals.cpp
		bstone_gog_content_path.cpp
		bstone_hw_video.cpp
		bstone_image_decoder.cpp
		bstone_image_encoder.cpp
		bstone_image_extractor.cpp
		bstone_logger.cpp
		bstone_memory_binary_reader.cpp
		bstone_memory_resource.cpp
		bstone_memory_stream.cpp
		bstone_mt_task_mgr.cpp
		bstone_nuked_opl3.cpp
		bstone_oal_audio_mixer.cpp
		bstone_oal_loader.cpp
		bstone_oal_resource.cpp
		bstone_oal_source.cpp
		bstone_oal_symbols.cpp
		bstone_opl3.cpp
		bstone_page_mgr.cpp
		bstone_pc_speaker_audio_decoder.cpp
		bstone_pcm_audio_decoder.cpp
		bstone_ps_fizzle_fx.cpp
		bstone_rgb8.cpp
		bstone_sdl_shared_library.cpp
		bstone_sha1.cpp
		bstone_sprite.cpp
		bstone_sprite_cache.cpp
		bstone_stb_image_decoder.cpp
		bstone_stb_image_encoder.cpp
		bstone_stb_image_utils.cpp
		bstone_steam_content_path.cpp
		bstone_stream.cpp
		bstone_string_helper.cpp
		bstone_string_view.cpp
		bstone_sw_video.cpp
		bstone_system_audio_mixer.cpp
		bstone_text_extractor.cpp
		bstone_text_reader.cpp
		bstone_text_writer.cpp
		bstone_time.cpp
		bstone_version.cpp
		bstone_video.cpp
		bstone_voice_group.cpp

		bstone_sys_audio_mgr.cpp
		bstone_sys_event_mgr.cpp
		bstone_sys_gl_context.cpp
		bstone_sys_gl_mgr.cpp
		bstone_sys_gl_shared_library.cpp
		bstone_sys_logger.cpp
		bstone_sys_mouse_mgr.cpp
		bstone_sys_push_audio_device.cpp
		bstone_sys_renderer.cpp
		bstone_sys_system_mgr.cpp
		bstone_sys_texture.cpp
		bstone_sys_texture_lock.cpp
		bstone_sys_video_mgr.cpp
		bstone_sys_window.cpp
		bstone_sys_window_mgr.cpp

		bstone_sys_sdl_audio_mgr.cpp
		bstone_sys_sdl_detail.cpp
		bstone_sys_sdl_push_audio_device.cpp
		bstone_sys_sdl_event_mgr.cpp
		bstone_sys_sdl_exception.cpp
		bstone_sys_sdl_gl_context.cpp
		bstone_sys_sdl_gl_mgr.cpp
		bstone_sys_sdl_gl_shared_library.cpp
		bstone_sys_sdl_message_box.cpp
		bstone_sys_sdl_mouse_mgr.cpp
		bstone_sys_sdl_renderer.cpp
		bstone_sys_sdl_system_mgr.cpp
		bstone_sys_sdl_texture.cpp
		bstone_sys_sdl_texture_lock.cpp
		bstone_sys_sdl_video_mgr.cpp
		bstone_sys_sdl_window.cpp
		bstone_sys_sdl_window_mgr.cpp
)

if (WIN32)
	target_sources(
		${PROJECT_NAME}
		PRIVATE
			"${PROJECT_BINARY_DIR}/sys/win32/bstone_win32.rc"
			sys/win32/bstone_win32_registry.cpp
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

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "Boris I. Bendovsky")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${BSTONE_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Blake Stone unofficial source port")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
