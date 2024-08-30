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
				-p $<SHELL_PATH:${PROJECT_SOURCE_DIR}/src>
		VERBATIM
	)
endif ()

#
# Configure files.
#
configure_file(
	"${PROJECT_SOURCE_DIR}/src/bstone_version_.h.in"
	"${PROJECT_BINARY_DIR}/src/bstone_version_.h"
)

if (WIN32)
	configure_file(
		"${PROJECT_SOURCE_DIR}/src/resources/win32/bstone_win32.rc.in"
		"${PROJECT_BINARY_DIR}/src/resources/win32/bstone_win32.rc"
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
		bstone::lib::khronos
		bstone::lib::nuked_opl3
		bstone::lib::openal_soft
		bstone::lib::stb
		bstone::lib::xbrz
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
		src
		src/dosbox
		src/resources/win32
		${PROJECT_BINARY_DIR}/src
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
			src/bstone_precompiled.h
	)
endif ()

# ===========================================================================

set(BSTONE_LEGACY_SOURCES
	src/3d_act1.cpp
	src/3d_act2.cpp
	src/3d_agent.cpp
	src/3d_debug.cpp
	src/3d_def.h
	src/3d_draw.cpp
	src/3d_draw2.cpp
	src/3d_game.cpp
	src/3d_inter.cpp
	src/3d_main.cpp
	src/3d_menu.cpp
	src/3d_menu.h
	src/3d_msgs.cpp
	src/3d_play.cpp
	src/3d_scale.cpp
	src/3d_state.cpp
	src/an_codes.h
	src/audio.h
	src/colormap.cpp
	src/d3_d2.cpp
	src/d3_d2.h
	src/d3_dr2.cpp
	src/gfxv.h
	src/id_ca.cpp
	src/id_ca.h
	src/id_heads.h
	src/id_in.cpp
	src/id_in.h
	src/id_sd.cpp
	src/id_sd.h
	src/id_us.cpp
	src/id_us.h
	src/id_us_1.cpp
	src/id_vh.cpp
	src/id_vh.h
	src/id_vl.cpp
	src/id_vl.h
	src/jm_cio.h
	src/jm_free.cpp
	src/jm_lzh.cpp
	src/jm_lzh.h
	src/jm_tp.cpp
	src/jm_tp.h
	src/jm_vl.h
	src/markhack.cpp
	src/movie.cpp
	src/movie.h
	src/scale.cpp
	src/stub.cpp
	src/vgapal.cpp
)

target_sources(bstone PRIVATE ${BSTONE_LEGACY_SOURCES})
source_group("legacy" FILES ${BSTONE_LEGACY_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_DOSBOX_SOURCES
	src/dosbox/adlib.h
	src/dosbox/dbopl.cpp
	src/dosbox/dbopl.h
	src/dosbox/dosbox.h
	src/dosbox/hardware.h
	src/dosbox/inout.h
	src/dosbox/mixer.cpp
	src/dosbox/mixer.h
	src/dosbox/pic.h
	src/dosbox/setup.h
)

target_sources(bstone PRIVATE ${BSTONE_LIB_DOSBOX_SOURCES})
source_group("lib/dosbox" FILES ${BSTONE_LIB_DOSBOX_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_LIB_MISC_SOURCES
	src/bstone_algorithm.cpp
	src/bstone_algorithm.h
	src/bstone_api_string.cpp
	src/bstone_api_string.h
	src/bstone_array.cpp
	src/bstone_array.h
	src/bstone_ascii.cpp
	src/bstone_ascii.h
	src/bstone_assert.cpp
	src/bstone_assert.h
	src/bstone_auto_arena_resource.cpp
	src/bstone_auto_arena_resource.h
	src/bstone_binary_reader.cpp
	src/bstone_binary_reader.h
	src/bstone_binary_writer.cpp
	src/bstone_binary_writer.h
	src/bstone_cgm_clip_space.cpp
	src/bstone_cgm_clip_space.h
	src/bstone_cgm_mat.cpp
	src/bstone_cgm_mat.h
	src/bstone_cgm_transform.cpp
	src/bstone_cgm_transform.h
	src/bstone_cgm_vec.cpp
	src/bstone_cgm_vec.h
	src/bstone_char_conv.cpp
	src/bstone_char_conv.h
	src/bstone_char_traits.cpp
	src/bstone_char_traits.h
	src/bstone_configurations.h
	src/bstone_crc32.cpp
	src/bstone_crc32.h
	src/bstone_cxx.cpp
	src/bstone_cxx.h
	src/bstone_endian.cpp
	src/bstone_endian.h
	src/bstone_entry_point.cpp
	src/bstone_entry_point.h
	src/bstone_enum_flags.h
	src/bstone_exception.cpp
	src/bstone_exception.h
	src/bstone_file.cpp
	src/bstone_file.h
	src/bstone_file_posix.cpp
	src/bstone_file_stream.cpp
	src/bstone_file_stream.h
	src/bstone_file_win32.cpp
	src/bstone_fixed_pool_resource.cpp
	src/bstone_fixed_pool_resource.h
	src/bstone_four_cc.cpp
	src/bstone_four_cc.h
	src/bstone_fs.cpp
	src/bstone_fs.h
	src/bstone_fs_posix.cpp
	src/bstone_fs_win32.cpp
	src/bstone_generic_pool_resource.cpp
	src/bstone_generic_pool_resource.h
	src/bstone_memory.cpp
	src/bstone_memory.h
	src/bstone_memory_binary_reader.cpp
	src/bstone_memory_binary_reader.h
	src/bstone_memory_pool_bitmap.cpp
	src/bstone_memory_pool_bitmap.h
	src/bstone_memory_resource.cpp
	src/bstone_memory_resource.h
	src/bstone_memory_stream.cpp
	src/bstone_memory_stream.h
	src/bstone_platform.h
	src/bstone_posix_string.cpp
	src/bstone_posix_string.h
	src/bstone_scope_exit.cpp
	src/bstone_scope_exit.h
	src/bstone_sha1.cpp
	src/bstone_sha1.h
	src/bstone_shared_library.cpp
	src/bstone_shared_library.h
	src/bstone_shared_library_posix.cpp
	src/bstone_shared_library_win32.cpp
	src/bstone_single_pool_resource.cpp
	src/bstone_single_pool_resource.h
	src/bstone_source_location.cpp
	src/bstone_source_location.h
	src/bstone_span.cpp
	src/bstone_span.h
	src/bstone_static_memory_stream.cpp
	src/bstone_static_memory_stream.h
	src/bstone_static_ro_memory_stream.cpp
	src/bstone_static_ro_memory_stream.h
	src/bstone_stream.cpp
	src/bstone_stream.h
	src/bstone_string.cpp
	src/bstone_string.h
	src/bstone_string_view.cpp
	src/bstone_string_view.cpp
	src/bstone_string_view.h
	src/bstone_string_view.h
	src/bstone_sys_special_path.cpp
	src/bstone_sys_special_path.h
	src/bstone_sys_special_path_sdl2.cpp
	src/bstone_ts_auto_arena_resource.cpp
	src/bstone_ts_auto_arena_resource.h
	src/bstone_type_traits.h
	src/bstone_unicode.cpp
	src/bstone_unicode.h
	src/bstone_unique_resource.cpp
	src/bstone_unique_resource.h
	src/bstone_utf.cpp
	src/bstone_utf.h
	src/bstone_utf16.cpp
	src/bstone_utf16.h
	src/bstone_utf8.cpp
	src/bstone_utf8.h
	src/bstone_utility.cpp
	src/bstone_utility.h
	src/bstone_uuid.cpp
	src/bstone_uuid.h
	src/bstone_uuid_unix.cpp
	src/bstone_uuid_win32.cpp
	src/bstone_win32_advapi32_symbols.cpp
	src/bstone_win32_advapi32_symbols.h
	src/bstone_win32_com.cpp
	src/bstone_win32_com.h
	src/bstone_win32_os_version.cpp
	src/bstone_win32_os_version.h
	src/bstone_win32_registry_key.cpp
	src/bstone_win32_registry_key.h
	src/bstone_win32_wstring.cpp
	src/bstone_win32_wstring.h
	src/bstone_win32_unique_resources.cpp
	src/bstone_win32_unique_resources.h
	src/bstone_zstring_view.cpp
	src/bstone_zstring_view.h
	src/bstone_process.h
	src/bstone_process_posix.cpp
	src/bstone_process_win32.cpp
)

target_sources(bstone PRIVATE ${BSTONE_LIB_MISC_SOURCES})
source_group("lib/misc" FILES ${BSTONE_LIB_MISC_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_SOURCES
	src/bstone_hw_shader_registry.cpp
	src/bstone_hw_shader_registry.h
	src/bstone_hw_texture_mgr.cpp
	src/bstone_hw_texture_mgr.h
	src/bstone_missing_sprite_64x64_image.cpp
	src/bstone_missing_sprite_64x64_image.h
	src/bstone_missing_wall_64x64_image.cpp
	src/bstone_missing_wall_64x64_image.h
	src/bstone_renderer_type.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_SOURCES})
source_group("bstone/renderer" FILES ${BSTONE_BSTONE_RENDERER_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_3D_SOURCES
	src/bstone_r3r.cpp
	src/bstone_r3r.h
	src/bstone_r3r_buffer.cpp
	src/bstone_r3r_buffer.h
	src/bstone_r3r_cmd_buffer.cpp
	src/bstone_r3r_cmd_buffer.h
	src/bstone_r3r_limits.cpp
	src/bstone_r3r_limits.h
	src/bstone_r3r_mgr.cpp
	src/bstone_r3r_mgr.h
	src/bstone_r3r_r2_texture.cpp
	src/bstone_r3r_r2_texture.h
	src/bstone_r3r_sampler.cpp
	src/bstone_r3r_sampler.h
	src/bstone_r3r_shader.cpp
	src/bstone_r3r_shader.h
	src/bstone_r3r_shader_stage.cpp
	src/bstone_r3r_shader_stage.h
	src/bstone_r3r_shader_var.cpp
	src/bstone_r3r_shader_var.h
	src/bstone_r3r_tests.h
	src/bstone_r3r_types.h
	src/bstone_r3r_utils.cpp
	src/bstone_r3r_utils.h
	src/bstone_r3r_vertex_input.cpp
	src/bstone_r3r_vertex_input.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_3D_SOURCES})
source_group("bstone/renderer/3d" FILES ${BSTONE_BSTONE_RENDERER_3D_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_RENDERER_3D_GL_SOURCES
	src/bstone_gl_r3r.cpp
	src/bstone_gl_r3r.h
	src/bstone_gl_r3r_api.cpp
	src/bstone_gl_r3r_api.h
	src/bstone_gl_r3r_buffer.cpp
	src/bstone_gl_r3r_buffer.h
	src/bstone_gl_r3r_context.cpp
	src/bstone_gl_r3r_context.h
	src/bstone_gl_r3r_device_features.h
	src/bstone_gl_r3r_error.cpp
	src/bstone_gl_r3r_error.h
	src/bstone_gl_r3r_extension_mgr.cpp
	src/bstone_gl_r3r_extension_mgr.h
	src/bstone_gl_r3r_r2_texture.cpp
	src/bstone_gl_r3r_r2_texture.h
	src/bstone_gl_r3r_sampler.cpp
	src/bstone_gl_r3r_sampler.h
	src/bstone_gl_r3r_sampler_mgr.cpp
	src/bstone_gl_r3r_sampler_mgr.h
	src/bstone_gl_r3r_shader.cpp
	src/bstone_gl_r3r_shader.h
	src/bstone_gl_r3r_shader_stage.cpp
	src/bstone_gl_r3r_shader_stage.h
	src/bstone_gl_r3r_shader_var.cpp
	src/bstone_gl_r3r_shader_var.h
	src/bstone_gl_r3r_utils.cpp
	src/bstone_gl_r3r_utils.h
	src/bstone_gl_r3r_version.h
	src/bstone_gl_r3r_vertex_input.cpp
	src/bstone_gl_r3r_vertex_input.h
	src/bstone_gl_r3r_vertex_input_mgr.cpp
	src/bstone_gl_r3r_vertex_input_mgr.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_RENDERER_3D_GL_SOURCES})
source_group("bstone/renderer/3d/gl" FILES ${BSTONE_BSTONE_RENDERER_3D_GL_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SOURCES
	src/bstone_sys_audio_mgr.cpp
	src/bstone_sys_audio_mgr.h
	src/bstone_sys_audio_mgr_null.cpp
	src/bstone_sys_audio_mgr_null.h
	src/bstone_sys_color.cpp
	src/bstone_sys_color.h
	src/bstone_sys_event.cpp
	src/bstone_sys_event.h
	src/bstone_sys_event_mgr.cpp
	src/bstone_sys_event_mgr.h
	src/bstone_sys_event_mgr_null.cpp
	src/bstone_sys_event_mgr_null.h
	src/bstone_sys_gl_context.cpp
	src/bstone_sys_gl_context.h
	src/bstone_sys_gl_context_attributes.h
	src/bstone_sys_gl_context_profile.cpp
	src/bstone_sys_gl_context_profile.h
	src/bstone_sys_gl_current_context.cpp
	src/bstone_sys_gl_current_context.h
	src/bstone_sys_gl_symbol_resolver.cpp
	src/bstone_sys_gl_symbol_resolver.h
	src/bstone_sys_keyboard_key.cpp
	src/bstone_sys_keyboard_key.h
	src/bstone_sys_logger.cpp
	src/bstone_sys_logger.h
	src/bstone_sys_message_box.cpp
	src/bstone_sys_message_box.h
	src/bstone_sys_mouse.cpp
	src/bstone_sys_mouse.h
	src/bstone_sys_mouse_mgr.cpp
	src/bstone_sys_mouse_mgr.h
	src/bstone_sys_pixel_format.h
	src/bstone_sys_polling_audio_device.cpp
	src/bstone_sys_polling_audio_device.h
	src/bstone_sys_rectangle.cpp
	src/bstone_sys_rectangle.h
	src/bstone_sys_renderer.cpp
	src/bstone_sys_renderer.h
	src/bstone_sys_swap_interval_type.cpp
	src/bstone_sys_swap_interval_type.h
	src/bstone_sys_system_mgr.cpp
	src/bstone_sys_system_mgr.h
	src/bstone_sys_texture.cpp
	src/bstone_sys_texture.h
	src/bstone_sys_texture_lock.cpp
	src/bstone_sys_texture_lock.h
	src/bstone_sys_video_mgr.cpp
	src/bstone_sys_video_mgr.h
	src/bstone_sys_video_mgr_null.cpp
	src/bstone_sys_video_mgr_null.h
	src/bstone_sys_window.cpp
	src/bstone_sys_window.h
	src/bstone_sys_window_mgr.cpp
	src/bstone_sys_window_mgr.h
	src/bstone_sys_window_rounded_corner_mgr.cpp
	src/bstone_sys_window_rounded_corner_mgr.h
	src/bstone_sys_window_rounded_corner_mgr_null.cpp
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SOURCES})
source_group("bstone/sys" FILES ${BSTONE_BSTONE_SYS_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SYS_SDL2_SOURCES
	src/bstone_sys_audio_mgr_sdl2.cpp
	src/bstone_sys_audio_mgr_sdl2.h
	src/bstone_sys_detail_sdl2.cpp
	src/bstone_sys_detail_sdl2.h
	src/bstone_sys_event_mgr_sdl2.cpp
	src/bstone_sys_event_mgr_sdl2.h
	src/bstone_sys_exception_sdl2.cpp
	src/bstone_sys_exception_sdl2.h
	src/bstone_sys_gl_context_sdl2.cpp
	src/bstone_sys_gl_context_sdl2.h
	src/bstone_sys_gl_current_context_sdl2.cpp
	src/bstone_sys_gl_current_context_sdl2.h
	src/bstone_sys_gl_symbol_resolver_sdl2.cpp
	src/bstone_sys_gl_symbol_resolver_sdl2.h
	src/bstone_sys_limits_sdl2.h
	src/bstone_sys_message_box_sdl2.cpp
	src/bstone_sys_mouse_mgr_sdl2.cpp
	src/bstone_sys_mouse_mgr_sdl2.h
	src/bstone_sys_pixel_format.cpp
	src/bstone_sys_polling_audio_device_sdl2.cpp
	src/bstone_sys_polling_audio_device_sdl2.h
	src/bstone_sys_renderer_sdl2.cpp
	src/bstone_sys_renderer_sdl2.h
	src/bstone_sys_sdl2_subsystem.cpp
	src/bstone_sys_sdl2_subsystem.h
	src/bstone_sys_system_mgr_sdl2.cpp
	src/bstone_sys_texture_lock_sdl2.cpp
	src/bstone_sys_texture_lock_sdl2.h
	src/bstone_sys_texture_sdl2.cpp
	src/bstone_sys_texture_sdl2.h
	src/bstone_sys_video_mgr_sdl2.cpp
	src/bstone_sys_video_mgr_sdl2.h
	src/bstone_sys_window_mgr_sdl2.cpp
	src/bstone_sys_window_mgr_sdl2.h
	src/bstone_sys_window_rounded_corner_mgr_sdl2.cpp
	src/bstone_sys_window_sdl2.cpp
	src/bstone_sys_window_sdl2.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SYS_SDL2_SOURCES})
source_group("bstone/sys/sdl2" FILES ${BSTONE_BSTONE_SYS_SDL2_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	src/bstone_audio_content_mgr.cpp
	src/bstone_audio_content_mgr.h
	src/bstone_audio_decoder.cpp
	src/bstone_audio_decoder.h
	src/bstone_audio_extractor.cpp
	src/bstone_audio_extractor.h
	src/bstone_audio_mixer.cpp
	src/bstone_audio_mixer.h
	src/bstone_audio_mixer_output_gains.h
	src/bstone_audio_mixer_utils.cpp
	src/bstone_audio_mixer_utils.h
	src/bstone_audio_mixer_validator.cpp
	src/bstone_audio_mixer_validator.h
	src/bstone_audio_mixer_voice_handle.cpp
	src/bstone_audio_mixer_voice_handle.h
	src/bstone_audio_mixer_voice_handle_mgr.cpp
	src/bstone_audio_mixer_voice_handle_mgr.h
	src/bstone_audio_sample_converter.cpp
	src/bstone_audio_sample_converter.h
	src/bstone_pc_speaker_audio_decoder.cpp
	src/bstone_pc_speaker_audio_decoder.h
	src/bstone_pcm_audio_decoder.cpp
	src/bstone_pcm_audio_decoder.h
	src/bstone_system_audio_mixer.cpp
	src/bstone_system_audio_mixer.h
	src/bstone_voice.h
	src/bstone_voice_group.cpp
	src/bstone_voice_group.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	src/bstone_adlib_decoder.cpp
	src/bstone_adlib_decoder.h
	src/bstone_adlib_music_decoder.cpp
	src/bstone_adlib_music_decoder.h
	src/bstone_adlib_sfx_decoder.cpp
	src/bstone_adlib_sfx_decoder.h
	src/bstone_dosbox_dbopl.cpp
	src/bstone_dosbox_dbopl.h
	src/bstone_nuked_opl3.cpp
	src/bstone_nuked_opl3.h
	src/bstone_opl3.cpp
	src/bstone_opl3.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio/opl" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_AUDIO_SOURCES
	src/bstone_oal_audio_mixer.cpp
	src/bstone_oal_audio_mixer.h
	src/bstone_oal_loader.cpp
	src/bstone_oal_loader.h
	src/bstone_oal_resource.cpp
	src/bstone_oal_resource.h
	src/bstone_oal_source.cpp
	src/bstone_oal_source.h
	src/bstone_oal_symbols.cpp
	src/bstone_oal_symbols.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_AUDIO_SOURCES})
source_group("bstone/audio/openal" FILES ${BSTONE_BSTONE_AUDIO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_VIDEO_SOURCES
	src/bstone_fizzle_fx.cpp
	src/bstone_fizzle_fx.h
	src/bstone_generic_fizzle_fx.cpp
	src/bstone_generic_fizzle_fx.h
	src/bstone_hw_video.cpp
	src/bstone_hw_video.h
	src/bstone_ps_fizzle_fx.cpp
	src/bstone_ps_fizzle_fx.h
	src/bstone_sw_video.cpp
	src/bstone_sw_video.h
	src/bstone_video.cpp
	src/bstone_video.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_VIDEO_SOURCES})
source_group("bstone/video" FILES ${BSTONE_BSTONE_VIDEO_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_IMAGE_SOURCES
	src/bstone_bmp_image_common.h
	src/bstone_bmp_image_decoder.cpp
	src/bstone_bmp_image_decoder.h
	src/bstone_image_decoder.cpp
	src/bstone_image_decoder.h
	src/bstone_image_encoder.cpp
	src/bstone_image_encoder.h
	src/bstone_image_extractor.cpp
	src/bstone_image_extractor.h
	src/bstone_sprite.cpp
	src/bstone_sprite.h
	src/bstone_sprite_cache.cpp
	src/bstone_sprite_cache.h
	src/bstone_stb_image_decoder.cpp
	src/bstone_stb_image_decoder.h
	src/bstone_stb_image_encoder.cpp
	src/bstone_stb_image_encoder.h
	src/bstone_stb_image_utils.cpp
	src/bstone_stb_image_utils.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_IMAGE_SOURCES})
source_group("bstone/image" FILES ${BSTONE_BSTONE_IMAGE_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_CVAR_SOURCES
	src/bstone_ccmd.cpp
	src/bstone_ccmd.h
	src/bstone_ccmd_action.cpp
	src/bstone_ccmd_action.h
	src/bstone_ccmd_mgr.cpp
	src/bstone_ccmd_mgr.h
	src/bstone_cvalidator.cpp
	src/bstone_cvalidator.h
	src/bstone_cvar.cpp
	src/bstone_cvar.h
	src/bstone_cvar_mgr.cpp
	src/bstone_cvar_mgr.h
	src/bstone_cvar_string.cpp
	src/bstone_cvar_string.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_CVAR_SOURCES})
source_group("bstone/cvar" FILES ${BSTONE_BSTONE_CVAR_SOURCES})

# ---------------------------------------------------------------------------

set(BSTONE_BSTONE_SOURCES
	src/bstone_archiver.cpp
	src/bstone_archiver.h
	src/bstone_atomic_flag.cpp
	src/bstone_atomic_flag.h
	src/bstone_char_hasher.cpp
	src/bstone_char_hasher.h
	src/bstone_cl.cpp
	src/bstone_cl.h
	src/bstone_configurations.cpp
	src/bstone_content_path.cpp
	src/bstone_content_path.h
	src/bstone_door.h
	src/bstone_encoding.cpp
	src/bstone_encoding.h
	src/bstone_exception_utils.cpp
	src/bstone_exception_utils.h
	src/bstone_extent_2d.cpp
	src/bstone_extent_2d.h
	src/bstone_format_string.cpp
	src/bstone_format_string.h
	src/bstone_fs_utils.cpp
	src/bstone_fs_utils.h
	src/bstone_game_timer.cpp
	src/bstone_game_timer.h
	src/bstone_globals.cpp
	src/bstone_globals.h
	src/bstone_gog_content_path.cpp
	src/bstone_level_extractor.cpp
	src/bstone_level_extractor.h
	src/bstone_logger.cpp
	src/bstone_logger.h
	src/bstone_math.h
	src/bstone_mod_value.h
	src/bstone_mt_task_mgr.cpp
	src/bstone_mt_task_mgr.h
	src/bstone_offset_2d.cpp
	src/bstone_offset_2d.h
	src/bstone_page_mgr.cpp
	src/bstone_page_mgr.h
	src/bstone_precompiled.h
	src/bstone_r2_extent.h
	src/bstone_r2_offset.h
	src/bstone_ref_values.h
	src/bstone_rgb8.cpp
	src/bstone_rgb8.h
	src/bstone_rlew_decoder.cpp
	src/bstone_rlew_decoder.h
	src/bstone_rgb_palette.h
	src/bstone_saved_game.cpp
	src/bstone_saved_game.h
	src/bstone_steam_content_path.cpp
	src/bstone_string_helper.cpp
	src/bstone_string_helper.h
	src/bstone_string_view_hasher.h
	src/bstone_text_extractor.cpp
	src/bstone_text_extractor.h
	src/bstone_text_reader.cpp
	src/bstone_text_reader.h
	src/bstone_text_writer.cpp
	src/bstone_text_writer.h
	src/bstone_time.cpp
	src/bstone_time.h
	src/bstone_version.cpp
	src/bstone_version.h
	src/bstone_zstring_view_hasher.h
)

target_sources(bstone PRIVATE ${BSTONE_BSTONE_SOURCES})
source_group("bstone" FILES ${BSTONE_BSTONE_SOURCES})

# ---------------------------------------------------------------------------

if (WIN32)
	set(BSTONE_RESOURCES_WIN32_SOURCES
		src/resources/win32/bstone_win32_rc.h
		$<$<CXX_COMPILER_ID:MSVC>:src/resources/win32/bstone_win32_msvc.manifest>
		"${PROJECT_BINARY_DIR}/src/resources/win32/bstone_win32.rc"
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

install(FILES ../../CHANGELOG.md
	DESTINATION "."
	RENAME bstone_changelog.txt
)

install(FILES ../../README.md
	DESTINATION "."
	RENAME bstone_readme.txt
)

install(FILES ../../LICENSE.txt
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
