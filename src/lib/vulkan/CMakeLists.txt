cmake_minimum_required(VERSION 3.5.0 FATAL_ERROR)
project(bstone_vulkan VERSION 1.0.0 LANGUAGES CXX)
message(STATUS "[bstone_vulkan] v${PROJECT_VERSION}")
add_library(bstone_vulkan STATIC "")
add_library(bstone::vulkan ALIAS bstone_vulkan)
target_compile_definitions(bstone_vulkan INTERFACE VK_NO_PROTOTYPES)
target_include_directories(bstone_vulkan INTERFACE include)
target_sources(bstone_vulkan PRIVATE
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
	#
	src/bstone_vulkan.cpp
)
