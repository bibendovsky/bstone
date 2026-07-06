cmake_minimum_required(VERSION 3.24.0 FATAL_ERROR)
project(bstone_lib_vulkan VERSION 1.0.0 LANGUAGES NONE)
message(STATUS "[${PROJECT_NAME}] v${PROJECT_VERSION}")
add_library(${PROJECT_NAME} INTERFACE)
add_library(bstone::lib::vulkan ALIAS ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} INTERFACE VK_NO_PROTOTYPES)
target_include_directories(${PROJECT_NAME} INTERFACE include)
target_sources(${PROJECT_NAME} PRIVATE
	include/vk_video/vulkan_video_codecs_common.h
	include/vk_video/vulkan_video_codec_av1std.h
	include/vk_video/vulkan_video_codec_av1std_decode.h
	include/vk_video/vulkan_video_codec_av1std_encode.h
	include/vk_video/vulkan_video_codec_h264std.h
	include/vk_video/vulkan_video_codec_h264std_decode.h
	include/vk_video/vulkan_video_codec_h264std_encode.h
	include/vk_video/vulkan_video_codec_h265std.h
	include/vk_video/vulkan_video_codec_h265std_decode.h
	include/vk_video/vulkan_video_codec_h265std_encode.h
	include/vk_video/vulkan_video_codec_vp9std.h
	include/vk_video/vulkan_video_codec_vp9std_decode.h
	#
	include/vulkan/vk_enum_string_helper.h
	include/vulkan/vk_icd.h
	include/vulkan/vk_layer.h
	include/vulkan/vk_platform.h
	include/vulkan/vulkan.h
	include/vulkan/vulkan_android.h
	include/vulkan/vulkan_beta.h
	include/vulkan/vulkan_core.h
	include/vulkan/vulkan_directfb.h
	include/vulkan/vulkan_fuchsia.h
	include/vulkan/vulkan_ggp.h
	include/vulkan/vulkan_ios.h
	include/vulkan/vulkan_macos.h
	include/vulkan/vulkan_metal.h
	include/vulkan/vulkan_ohos.h
	include/vulkan/vulkan_screen.h
	include/vulkan/vulkan_vi.h
	include/vulkan/vulkan_wayland.h
	include/vulkan/vulkan_win32.h
	include/vulkan/vulkan_xcb.h
	include/vulkan/vulkan_xlib.h
	include/vulkan/vulkan_xlib_xrandr.h
)
