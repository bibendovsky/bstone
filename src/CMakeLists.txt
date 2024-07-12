#[[
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
]]

cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone VERSION 1.2.13 LANGUAGES CXX)

#
# Version.
#
set(BSTONE_VERSION "${PROJECT_VERSION}")
set(BSTONE_VERSION_PRE_RELEASE "wip")

if (NOT ("${BSTONE_VERSION_PRE_RELEASE}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}-${BSTONE_VERSION_PRE_RELEASE}")
endif ()

set(BSTONE_VERSION_SHORT "${BSTONE_VERSION}")

if (NOT ("${BSTONE_GIT_HASH}" STREQUAL ""))
	set(BSTONE_VERSION "${BSTONE_VERSION}+${BSTONE_GIT_HASH}")
endif ()

if (NOT ("${BSTONE_GIT_HASH_SHORT}" STREQUAL ""))
	set(BSTONE_VERSION_SHORT "${BSTONE_VERSION_SHORT}+${BSTONE_GIT_HASH_SHORT}")
endif ()

message(STATUS "[bstone] v${BSTONE_VERSION}")

#
# Options.
#
option(BSTONE_USE_STATIC_LINKING "Link build statically." OFF)
option(BSTONE_PANDORA "Build for Pandora." OFF)
option(BSTONE_USE_PCH "Use precompiled headers if available." OFF)
option(BSTONE_USE_MULTI_PROCESS_COMPILATION "Use multi-process compilation." ON)
option(BSTONE_MORE_COMPILER_WARNINGS "Enables more compiler warnings." OFF)
option(BSTONE_NO_ASSERT "Disable assertion." OFF)

# SDL2W
#
set(BSTONE_SDL_COMPONENTS "")

if (NOT BSTONE_INTERNAL_SDL2)
	if (BSTONE_USE_STATIC_LINKING)
		set(BSTONE_SDL_COMPONENTS "${BSTONE_SDL_COMPONENTS};static")
	endif ()

	find_package(SDL2W 2.0.4 REQUIRED COMPONENTS ${BSTONE_SDL_COMPONENTS})
endif ()

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
add_executable(bstone "")

add_custom_command(
	TARGET bstone POST_BUILD
	COMMAND $<$<CONFIG:RELEASE>:${CMAKE_STRIP}> $<$<CONFIG:RELEASE>:$<TARGET_FILE:bstone>>
	VERBATIM
)

if (BSTONE_TRIM_FILE_PATHS_IN_EXE)
	add_dependencies(bstone bstone_ltrimcs)

	add_custom_command(
		TARGET bstone POST_BUILD
		COMMAND
			$<TARGET_FILE:bstone_ltrimcs>
				-f $<SHELL_PATH:$<TARGET_FILE:bstone>>
				-p $<SHELL_PATH:${PROJECT_SOURCE_DIR}>
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
		"${PROJECT_SOURCE_DIR}/resources/win32/bstone_win32.rc.in"
		"${PROJECT_BINARY_DIR}/resources/win32/bstone_win32.rc"
	)
endif ()

#
# Target properties.
#
set_target_properties(bstone PROPERTIES
	CXX_STANDARD 14
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

if (WIN32)
	set_target_properties(bstone PROPERTIES
		WIN32_EXECUTABLE TRUE
	)
endif ()

#
# Target compile definitions.
#
target_compile_definitions(bstone
	PRIVATE
		$<$<BOOL:${WIN32}>:UNICODE>
		$<$<BOOL:${WIN32}>:_UNICODE>
		$<$<BOOL:${WIN32}>:_CRT_SECURE_NO_WARNINGS>
		$<$<BOOL:${WIN32}>:_SCL_SECURE_NO_WARNINGS>
		$<$<BOOL:${WIN32}>:_BIND_TO_CURRENT_VCLIBS_VERSION=1>
		$<$<BOOL:${WIN32}>:NTDDI_VERSION=0x05010000>
		$<$<BOOL:${WIN32}>:_WIN32_WINNT=0x0501>
		$<$<BOOL:${WIN32}>:WINVER=0x0501>
		$<$<BOOL:${BSTONE_NO_ASSERT}>:BSTONE_NO_ASSERT>
)

if (BSTONE_PANDORA)
	target_compile_definitions(bstone
		PRIVATE
			BSTONE_PANDORA
	)
endif ()

#
# Target compile options.
#
if (BSTONE_MORE_COMPILER_WARNINGS)
	target_compile_options(bstone
		PRIVATE
			$<$<CXX_COMPILER_ID:MSVC>:/W4>
			$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic>
	)
endif ()

if (MSVC)
	if (BSTONE_USE_STATIC_LINKING)
		target_compile_options(bstone
			PRIVATE
				$<$<CONFIG:DEBUG>:-MTd>
				$<$<NOT:$<CONFIG:DEBUG>>:-MT>
		)
	endif ()

	if (BSTONE_USE_MULTI_PROCESS_COMPILATION)
		target_compile_options(bstone
			PRIVATE
				-MP
		)
	endif ()
endif ()

if (BSTONE_PANDORA)
	target_compile_options(bstone
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
		target_link_libraries(bstone
			PRIVATE
				$<$<CXX_COMPILER_ID:Clang>:-static>
				$<$<CXX_COMPILER_ID:GNU>:-static>
		)
	else ()
		target_link_libraries(bstone
			PRIVATE
				$<$<CXX_COMPILER_ID:Clang>:-static>
				$<$<CXX_COMPILER_ID:GNU>:-static-libgcc>
				$<$<CXX_COMPILER_ID:GNU>:-static-libstdc++>
		)
	endif ()
endif ()

target_link_libraries(bstone
	PRIVATE
		${CMAKE_DL_LIBS}
		$<$<BOOL:${BSTONE_INTERNAL_SDL2}>:SDL2::SDL2-static>
		$<$<NOT:$<BOOL:${BSTONE_INTERNAL_SDL2}>>:SDL2W::SDL2W>
		$<$<BOOL:${WIN32}>:wbemuuid>
		Threads::Threads
)

if (BSTONE_PANDORA)
	target_link_libraries(bstone
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
target_include_directories(bstone
	PRIVATE
		.
		dosbox
		lib/khronos
		lib/openal_soft/include/AL
		lib/stb
		lib/xbrz
		resources/win32
		${PROJECT_BINARY_DIR}
)

if (BSTONE_PANDORA)
	target_include_directories(bstone
		PRIVATE
			$ENV{PNDSDK}
	)
endif ()

if (BSTONE_USE_PCH AND NOT (${CMAKE_VERSION} VERSION_LESS "3.16.0"))
	target_precompile_headers(bstone
		PRIVATE
			bstone_precompiled.h
	)
endif ()

# ===========================================================================

set(BSTONE_LEGACY_SOURCES
	3d_act1.cpp
	3d_act2.cpp
	3d_agent.cpp
	3d_debug.cpp
	3d_def.h
	3d_draw.cpp
	3d_draw2.cpp
	3d_game.cpp
	3d_inter.cpp
	3d_main.cpp
	3d_menu.cpp
	3d_menu.h
	3d_msgs.cpp
	3d_play.cpp
	3d_scale.cpp
	3d_state.cpp
	an_codes.h
	audio.h
	colormap.cpp
	d3_d2.cpp
	d3_d2.h
	d3_dr2.cpp
	gfxv.h
	id_ca.cpp
	id_ca.h
	id_heads.h
	id_in.cpp
	id_in.h
	id_sd.cpp
	id_sd.h
	id_us.cpp
	id_us.h
	id_us_1.cpp
	id_vh.cpp
	id_vh.h
	id_vl.cpp
	id_vl.h
	jm_cio.h
	jm_free.cpp
	jm_lzh.cpp
	jm_lzh.h
	jm_tp.cpp
	jm_tp.h
	jm_vl.h
	markhack.cpp
	movie.cpp
	movie.h
	scale.cpp
	stub.cpp
	vgapal.cpp
)

target_sources(bstone PRIVATE ${BSTONE_LEGACY_SOURCES})
source_group("legacy" FILES ${BSTONE_LEGACY_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_DOSBOX_SOURCES
	dosbox/adlib.h
	dosbox/dbopl.cpp
	dosbox/dbopl.h
	dosbox/dosbox.h
	dosbox/hardware.h
	dosbox/inout.h
	dosbox/mixer.cpp
	dosbox/mixer.h
	dosbox/pic.h
	dosbox/setup.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_DOSBOX_SOURCES})
source_group("lib/dosbox" FILES ${BSTONE_LIB_DOSBOX_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_XBRZ_SOURCES
	lib/xbrz/xbrz.cpp
	lib/xbrz/xbrz.h
	lib/xbrz/xbrz_config.h
	lib/xbrz/xbrz_tools.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_XBRZ_SOURCES})
source_group("lib/xbrz" FILES ${BSTONE_LIB_XBRZ_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_MISC_SOURCES
	bstone_algorithm.cpp
	bstone_algorithm.h
	bstone_api_string.cpp
	bstone_api_string.h
	bstone_array.cpp
	bstone_array.h
	bstone_ascii.cpp
	bstone_ascii.h
	bstone_assert.cpp
	bstone_assert.h
	bstone_auto_arena_resource.cpp
	bstone_auto_arena_resource.h
	bstone_binary_reader.cpp
	bstone_binary_reader.h
	bstone_binary_writer.cpp
	bstone_binary_writer.h
	bstone_cgm_clip_space.cpp
	bstone_cgm_clip_space.h
	bstone_cgm_mat.cpp
	bstone_cgm_mat.h
	bstone_cgm_transform.cpp
	bstone_cgm_transform.h
	bstone_cgm_vec.cpp
	bstone_cgm_vec.h
	bstone_char_conv.cpp
	bstone_char_conv.h
	bstone_char_traits.cpp
	bstone_char_traits.h
	bstone_configurations.h
	bstone_crc32.cpp
	bstone_crc32.h
	bstone_cxx.cpp
	bstone_cxx.h
	bstone_endian.cpp
	bstone_endian.h
	bstone_entry_point.cpp
	bstone_entry_point.h
	bstone_enum_flags.h
	bstone_exception.cpp
	bstone_exception.h
	bstone_file.cpp
	bstone_file.h
	bstone_file_posix.cpp
	bstone_file_stream.cpp
	bstone_file_stream.h
	bstone_file_win32.cpp
	bstone_fixed_pool_resource.cpp
	bstone_fixed_pool_resource.h
	bstone_four_cc.cpp
	bstone_four_cc.h
	bstone_fs.cpp
	bstone_fs.h
	bstone_fs_posix.cpp
	bstone_fs_win32.cpp
	bstone_generic_pool_resource.cpp
	bstone_generic_pool_resource.h
	bstone_memory.cpp
	bstone_memory.h
	bstone_memory_binary_reader.cpp
	bstone_memory_binary_reader.h
	bstone_memory_pool_bitmap.cpp
	bstone_memory_pool_bitmap.h
	bstone_memory_resource.cpp
	bstone_memory_resource.h
	bstone_memory_stream.cpp
	bstone_memory_stream.h
	bstone_platform.h
	bstone_posix_string.cpp
	bstone_posix_string.h
	bstone_scope_exit.cpp
	bstone_scope_exit.h
	bstone_sha1.cpp
	bstone_sha1.h
	bstone_shared_library.cpp
	bstone_shared_library.h
	bstone_shared_library_posix.cpp
	bstone_shared_library_win32.cpp
	bstone_single_pool_resource.cpp
	bstone_single_pool_resource.h
	bstone_source_location.cpp
	bstone_source_location.h
	bstone_span.cpp
	bstone_span.h
	bstone_static_memory_stream.cpp
	bstone_static_memory_stream.h
	bstone_static_ro_memory_stream.cpp
	bstone_static_ro_memory_stream.h
	bstone_stream.cpp
	bstone_stream.h
	bstone_string.cpp
	bstone_string.h
	bstone_string_view.cpp
	bstone_string_view.cpp
	bstone_string_view.h
	bstone_string_view.h
	bstone_sys_special_path.cpp
	bstone_sys_special_path.h
	bstone_sys_special_path_sdl2.cpp
	bstone_ts_auto_arena_resource.cpp
	bstone_ts_auto_arena_resource.h
	bstone_type_traits.h
	bstone_unicode.cpp
	bstone_unicode.h
	bstone_unique_resource.cpp
	bstone_unique_resource.h
	bstone_utf.cpp
	bstone_utf.h
	bstone_utf16.cpp
	bstone_utf16.h
	bstone_utf8.cpp
	bstone_utf8.h
	bstone_utility.cpp
	bstone_utility.h
	bstone_uuid.cpp
	bstone_uuid.h
	bstone_uuid_unix.cpp
	bstone_uuid_win32.cpp
	bstone_win32_advapi32_symbols.cpp
	bstone_win32_advapi32_symbols.h
	bstone_win32_com.cpp
	bstone_win32_com.h
	bstone_win32_os_version.cpp
	bstone_win32_os_version.h
	bstone_win32_registry_key.cpp
	bstone_win32_registry_key.h
	bstone_win32_wstring.cpp
	bstone_win32_wstring.h
	bstone_zstring_view.cpp
	bstone_zstring_view.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_MISC_SOURCES})
source_group("lib/misc" FILES ${BSTONE_LIB_MISC_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_SOURCES
	bstone_hw_shader_registry.cpp
	bstone_hw_shader_registry.h
	bstone_hw_texture_mgr.cpp
	bstone_hw_texture_mgr.h
	bstone_missing_sprite_64x64_image.cpp
	bstone_missing_sprite_64x64_image.h
	bstone_missing_wall_64x64_image.cpp
	bstone_missing_wall_64x64_image.h
	bstone_renderer_type.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_SOURCES})
source_group("bstone/renderer" FILES ${BSTONE_BSTONE_RENDERER_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_3D_SOURCES
	bstone_r3r.cpp
	bstone_r3r.h
	bstone_r3r_buffer.cpp
	bstone_r3r_buffer.h
	bstone_r3r_cmd_buffer.cpp
	bstone_r3r_cmd_buffer.h
	bstone_r3r_limits.cpp
	bstone_r3r_limits.h
	bstone_r3r_mgr.cpp
	bstone_r3r_mgr.h
	bstone_r3r_r2_texture.cpp
	bstone_r3r_r2_texture.h
	bstone_r3r_sampler.cpp
	bstone_r3r_sampler.h
	bstone_r3r_shader.cpp
	bstone_r3r_shader.h
	bstone_r3r_shader_stage.cpp
	bstone_r3r_shader_stage.h
	bstone_r3r_shader_var.cpp
	bstone_r3r_shader_var.h
	bstone_r3r_tests.h
	bstone_r3r_types.h
	bstone_r3r_utils.cpp
	bstone_r3r_utils.h
	bstone_r3r_vertex_input.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_3D_SOURCES})
source_group("bstone/renderer/3d" FILES ${BSTONE_BSTONE_RENDERER_3D_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_3D_GL_SOURCES
	bstone_gl_r3r.cpp
	bstone_gl_r3r.h
	bstone_gl_r3r_api.cpp
	bstone_gl_r3r_api.h
	bstone_gl_r3r_buffer.cpp
	bstone_gl_r3r_buffer.h
	bstone_gl_r3r_context.cpp
	bstone_gl_r3r_context.h
	bstone_gl_r3r_device_features.h
	bstone_gl_r3r_error.cpp
	bstone_gl_r3r_error.h
	bstone_gl_r3r_extension_mgr.cpp
	bstone_gl_r3r_extension_mgr.h
	bstone_gl_r3r_r2_texture.cpp
	bstone_gl_r3r_r2_texture.h
	bstone_gl_r3r_sampler.cpp
	bstone_gl_r3r_sampler.h
	bstone_gl_r3r_sampler_mgr.cpp
	bstone_gl_r3r_sampler_mgr.h
	bstone_gl_r3r_shader.cpp
	bstone_gl_r3r_shader.h
	bstone_gl_r3r_shader_stage.cpp
	bstone_gl_r3r_shader_stage.h
	bstone_gl_r3r_shader_var.cpp
	bstone_gl_r3r_shader_var.h
	bstone_gl_r3r_utils.cpp
	bstone_gl_r3r_utils.h
	bstone_gl_r3r_version.h
	bstone_gl_r3r_vertex_input.cpp
	bstone_gl_r3r_vertex_input.h
	bstone_gl_r3r_vertex_input_mgr.cpp
	bstone_gl_r3r_vertex_input_mgr.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_3D_GL_SOURCES})
source_group("bstone/renderer/3d/gl" FILES ${BSTONE_BSTONE_RENDERER_3D_GL_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SOURCES
	bstone_sys_audio_mgr.cpp
	bstone_sys_audio_mgr.h
	bstone_sys_audio_mgr_null.cpp
	bstone_sys_audio_mgr_null.h
	bstone_sys_color.cpp
	bstone_sys_color.h
	bstone_sys_event.cpp
	bstone_sys_event.h
	bstone_sys_event_mgr.cpp
	bstone_sys_event_mgr.h
	bstone_sys_event_mgr_null.cpp
	bstone_sys_event_mgr_null.h
	bstone_sys_gl_context.cpp
	bstone_sys_gl_context.h
	bstone_sys_gl_context_attributes.h
	bstone_sys_gl_context_profile.cpp
	bstone_sys_gl_context_profile.h
	bstone_sys_gl_current_context.cpp
	bstone_sys_gl_current_context.h
	bstone_sys_gl_symbol_resolver.cpp
	bstone_sys_gl_symbol_resolver.h
	bstone_sys_keyboard_key.cpp
	bstone_sys_keyboard_key.h
	bstone_sys_logger.cpp
	bstone_sys_logger.h
	bstone_sys_message_box.cpp
	bstone_sys_message_box.h
	bstone_sys_mouse.cpp
	bstone_sys_mouse.h
	bstone_sys_mouse_mgr.cpp
	bstone_sys_mouse_mgr.h
	bstone_sys_pixel_format.h
	bstone_sys_polling_audio_device.cpp
	bstone_sys_polling_audio_device.h
	bstone_sys_rectangle.cpp
	bstone_sys_rectangle.h
	bstone_sys_renderer.cpp
	bstone_sys_renderer.h
	bstone_sys_swap_interval_type.cpp
	bstone_sys_swap_interval_type.h
	bstone_sys_system_mgr.cpp
	bstone_sys_system_mgr.h
	bstone_sys_texture.cpp
	bstone_sys_texture.h
	bstone_sys_texture_lock.cpp
	bstone_sys_texture_lock.h
	bstone_sys_video_mgr.cpp
	bstone_sys_video_mgr.h
	bstone_sys_video_mgr_null.cpp
	bstone_sys_video_mgr_null.h
	bstone_sys_window.cpp
	bstone_sys_window.h
	bstone_sys_window_mgr.cpp
	bstone_sys_window_mgr.h
	bstone_sys_window_rounded_corner_mgr.cpp
	bstone_sys_window_rounded_corner_mgr.h
	bstone_sys_window_rounded_corner_mgr_null.cpp
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SOURCES})
source_group("bstone/sys" FILES ${BSTONE_BSTONE_SYS_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SDL2_SOURCES
	bstone_sys_audio_mgr_sdl2.cpp
	bstone_sys_audio_mgr_sdl2.h
	bstone_sys_detail_sdl2.cpp
	bstone_sys_detail_sdl2.h
	bstone_sys_event_mgr_sdl2.cpp
	bstone_sys_event_mgr_sdl2.h
	bstone_sys_exception_sdl2.cpp
	bstone_sys_exception_sdl2.h
	bstone_sys_gl_context_sdl2.cpp
	bstone_sys_gl_context_sdl2.h
	bstone_sys_gl_current_context_sdl2.cpp
	bstone_sys_gl_current_context_sdl2.h
	bstone_sys_gl_symbol_resolver_sdl2.cpp
	bstone_sys_gl_symbol_resolver_sdl2.h
	bstone_sys_limits_sdl2.h
	bstone_sys_message_box_sdl2.cpp
	bstone_sys_mouse_mgr_sdl2.cpp
	bstone_sys_mouse_mgr_sdl2.h
	bstone_sys_pixel_format.cpp
	bstone_sys_push_audio_device_sdl2.cpp
	bstone_sys_push_audio_device_sdl2.h
	bstone_sys_renderer_sdl2.cpp
	bstone_sys_renderer_sdl2.h
	bstone_sys_sdl2_subsystem.cpp
	bstone_sys_sdl2_subsystem.h
	bstone_sys_system_mgr_sdl2.cpp
	bstone_sys_texture_lock_sdl2.cpp
	bstone_sys_texture_lock_sdl2.h
	bstone_sys_texture_sdl2.cpp
	bstone_sys_texture_sdl2.h
	bstone_sys_video_mgr_sdl2.cpp
	bstone_sys_video_mgr_sdl2.h
	bstone_sys_window_mgr_sdl2.cpp
	bstone_sys_window_mgr_sdl2.h
	bstone_sys_window_rounded_corner_mgr_sdl2.cpp
	bstone_sys_window_sdl2.cpp
	bstone_sys_window_sdl2.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SDL2_SOURCES})
source_group("bstone/sys/sdl2" FILES ${BSTONE_BSTONE_SYS_SDL2_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	bstone_audio_content_mgr.cpp
	bstone_audio_content_mgr.h
	bstone_audio_decoder.cpp
	bstone_audio_decoder.h
	bstone_audio_extractor.cpp
	bstone_audio_extractor.h
	bstone_audio_mixer.cpp
	bstone_audio_mixer.h
	bstone_audio_mixer_output_gains.h
	bstone_audio_mixer_utils.cpp
	bstone_audio_mixer_utils.h
	bstone_audio_mixer_validator.cpp
	bstone_audio_mixer_validator.h
	bstone_audio_mixer_voice_handle.cpp
	bstone_audio_mixer_voice_handle.h
	bstone_audio_mixer_voice_handle_mgr.cpp
	bstone_audio_mixer_voice_handle_mgr.h
	bstone_audio_sample_converter.cpp
	bstone_audio_sample_converter.h
	bstone_pc_speaker_audio_decoder.cpp
	bstone_pc_speaker_audio_decoder.h
	bstone_pcm_audio_decoder.cpp
	bstone_pcm_audio_decoder.h
	bstone_system_audio_mixer.cpp
	bstone_system_audio_mixer.h
	bstone_voice.h
	bstone_voice_group.cpp
	bstone_voice_group.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	bstone_adlib_decoder.cpp
	bstone_adlib_decoder.h
	bstone_adlib_music_decoder.cpp
	bstone_adlib_music_decoder.h
	bstone_adlib_sfx_decoder.cpp
	bstone_adlib_sfx_decoder.h
	bstone_dosbox_dbopl.cpp
	bstone_dosbox_dbopl.h
	bstone_nuked_opl3.cpp
	bstone_nuked_opl3.h
	bstone_opl3.cpp
	bstone_opl3.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio/opl" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	bstone_oal_audio_mixer.cpp
	bstone_oal_audio_mixer.h
	bstone_oal_loader.cpp
	bstone_oal_loader.h
	bstone_oal_resource.cpp
	bstone_oal_resource.h
	bstone_oal_source.cpp
	bstone_oal_source.h
	bstone_oal_symbols.cpp
	bstone_oal_symbols.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio/openal" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_VIDEO_SOURCES
	bstone_fizzle_fx.cpp
	bstone_fizzle_fx.h
	bstone_generic_fizzle_fx.cpp
	bstone_generic_fizzle_fx.h
	bstone_hw_video.cpp
	bstone_hw_video.h
	bstone_ps_fizzle_fx.cpp
	bstone_ps_fizzle_fx.h
	bstone_sw_video.cpp
	bstone_sw_video.h
	bstone_video.cpp
	bstone_video.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_VIDEO_SOURCES})
source_group("bstone/video" FILES ${BSTONE_BSTONE_VIDEO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_IMAGE_SOURCES
	bstone_bmp_image_common.h
	bstone_bmp_image_decoder.cpp
	bstone_bmp_image_decoder.h
	bstone_image_decoder.cpp
	bstone_image_decoder.h
	bstone_image_encoder.cpp
	bstone_image_encoder.h
	bstone_image_extractor.cpp
	bstone_image_extractor.h
	bstone_sprite.cpp
	bstone_sprite.h
	bstone_sprite_cache.cpp
	bstone_sprite_cache.h
	bstone_stb_image_decoder.cpp
	bstone_stb_image_decoder.h
	bstone_stb_image_encoder.cpp
	bstone_stb_image_encoder.h
	bstone_stb_image_utils.cpp
	bstone_stb_image_utils.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_IMAGE_SOURCES})
source_group("bstone/image" FILES ${BSTONE_BSTONE_IMAGE_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_CVAR_SOURCES
	bstone_ccmd.cpp
	bstone_ccmd.h
	bstone_ccmd_action.cpp
	bstone_ccmd_action.h
	bstone_ccmd_mgr.cpp
	bstone_ccmd_mgr.h
	bstone_cvalidator.cpp
	bstone_cvalidator.h
	bstone_cvar.cpp
	bstone_cvar.h
	bstone_cvar_mgr.cpp
	bstone_cvar_mgr.h
	bstone_cvar_string.cpp
	bstone_cvar_string.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_CVAR_SOURCES})
source_group("bstone/cvar" FILES ${BSTONE_BSTONE_CVAR_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SOURCES
	bstone_archiver.cpp
	bstone_archiver.h
	bstone_atomic_flag.cpp
	bstone_atomic_flag.h
	bstone_char_hasher.cpp
	bstone_char_hasher.h
	bstone_cl.cpp
	bstone_cl.h
	bstone_configurations.cpp
	bstone_content_path.cpp
	bstone_content_path.h
	bstone_door.h
	bstone_encoding.cpp
	bstone_encoding.h
	bstone_exception_utils.cpp
	bstone_exception_utils.h
	bstone_extent_2d.cpp
	bstone_extent_2d.h
	bstone_format_string.cpp
	bstone_format_string.h
	bstone_fs_utils.cpp
	bstone_fs_utils.h
	bstone_game_timer.cpp
	bstone_game_timer.h
	bstone_globals.cpp
	bstone_globals.h
	bstone_gog_content_path.cpp
	bstone_level_extractor.cpp
	bstone_level_extractor.h
	bstone_logger.cpp
	bstone_logger.h
	bstone_math.h
	bstone_mod_value.h
	bstone_mt_task_mgr.cpp
	bstone_mt_task_mgr.h
	bstone_offset_2d.cpp
	bstone_offset_2d.h
	bstone_page_mgr.cpp
	bstone_page_mgr.h
	bstone_precompiled.h
	bstone_r2_extent.h
	bstone_r2_offset.h
	bstone_ref_values.h
	bstone_rgb8.cpp
	bstone_rgb8.h
	bstone_rlew_decoder.cpp
	bstone_rlew_decoder.h
	bstone_rgb_palette.h
	bstone_saved_game.cpp
	bstone_saved_game.h
	bstone_steam_content_path.cpp
	bstone_string_helper.cpp
	bstone_string_helper.h
	bstone_string_view_hasher.h
	bstone_text_extractor.cpp
	bstone_text_extractor.h
	bstone_text_reader.cpp
	bstone_text_reader.h
	bstone_text_writer.cpp
	bstone_text_writer.h
	bstone_time.cpp
	bstone_time.h
	bstone_version.cpp
	bstone_version.h
	bstone_zstring_view_hasher.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SOURCES})
source_group("bstone" FILES ${BSTONE_BSTONE_SOURCES})

# ---------------------------------------------------------------------------

if (WIN32)
	set(BSTONE_RESOURCES_WIN32_SOURCES
		resources/win32/bstone_win32_rc.h
		$<$<CXX_COMPILER_ID:MSVC>:resources/win32/bstone_win32_msvc.manifest>
		"${PROJECT_BINARY_DIR}/resources/win32/bstone_win32.rc"
	)

	target_sources(bstone PRIVATE ${BSTONE_RESOURCES_WIN32_SOURCES})
	source_group("resources/win32" FILES ${BSTONE_RESOURCES_WIN32_SOURCES})
endif ()

# ===========================================================================

#
# Install.
#
install(TARGETS bstone
	RUNTIME DESTINATION "."
)

install(FILES ../CHANGELOG.md
	DESTINATION "."
	RENAME bstone_changelog.txt
)

install(FILES ../README.md
	DESTINATION "."
	RENAME bstone_readme.txt
)

install(FILES ../LICENSE
	DESTINATION "."
	RENAME bstone_license.txt
)

set(CPACK_PACKAGE_NAME "bstone")
set(CPACK_PACKAGE_VENDOR "Boris I. Bendovsky")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})
set(CPACK_PACKAGE_VERSION ${BSTONE_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Blake Stone unofficial source port")
set(CPACK_GENERATOR "ZIP")
set(CPACK_SOURCE_GENERATOR "ZIP")
include(CPack)
