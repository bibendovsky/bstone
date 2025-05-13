/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Outputs various information about Vulkan.

#include "bstone_vk_r3r_info.h"
#include "vulkan/vulkan.h"
#include "bstone_ascii.h"
#include "bstone_assert.h"
#include "bstone_char_conv.h"
#include "bstone_uuid.h"
#include "bstone_sys_logger.h"
#include "bstone_vk_r3r_array_extractor.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_enum_strings.h"
#include <cstdint>
#include <cstdio>
#include <string>
#include <type_traits>

namespace bstone {

namespace {

constexpr VkFormat vk_r3r_info_impl_vk_formats[] = {
	VK_FORMAT_R4G4_UNORM_PACK8,
	VK_FORMAT_R4G4B4A4_UNORM_PACK16,
	VK_FORMAT_B4G4R4A4_UNORM_PACK16,
	VK_FORMAT_R5G6B5_UNORM_PACK16,
	VK_FORMAT_B5G6R5_UNORM_PACK16,
	VK_FORMAT_R5G5B5A1_UNORM_PACK16,
	VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	VK_FORMAT_A1R5G5B5_UNORM_PACK16,
	VK_FORMAT_R8_UNORM,
	VK_FORMAT_R8_SNORM,
	VK_FORMAT_R8_USCALED,
	VK_FORMAT_R8_SSCALED,
	VK_FORMAT_R8_UINT,
	VK_FORMAT_R8_SINT,
	VK_FORMAT_R8_SRGB,
	VK_FORMAT_R8G8_UNORM,
	VK_FORMAT_R8G8_SNORM,
	VK_FORMAT_R8G8_USCALED,
	VK_FORMAT_R8G8_SSCALED,
	VK_FORMAT_R8G8_UINT,
	VK_FORMAT_R8G8_SINT,
	VK_FORMAT_R8G8_SRGB,
	VK_FORMAT_R8G8B8_UNORM,
	VK_FORMAT_R8G8B8_SNORM,
	VK_FORMAT_R8G8B8_USCALED,
	VK_FORMAT_R8G8B8_SSCALED,
	VK_FORMAT_R8G8B8_UINT,
	VK_FORMAT_R8G8B8_SINT,
	VK_FORMAT_R8G8B8_SRGB,
	VK_FORMAT_B8G8R8_UNORM,
	VK_FORMAT_B8G8R8_SNORM,
	VK_FORMAT_B8G8R8_USCALED,
	VK_FORMAT_B8G8R8_SSCALED,
	VK_FORMAT_B8G8R8_UINT,
	VK_FORMAT_B8G8R8_SINT,
	VK_FORMAT_B8G8R8_SRGB,
	VK_FORMAT_R8G8B8A8_UNORM,
	VK_FORMAT_R8G8B8A8_SNORM,
	VK_FORMAT_R8G8B8A8_USCALED,
	VK_FORMAT_R8G8B8A8_SSCALED,
	VK_FORMAT_R8G8B8A8_UINT,
	VK_FORMAT_R8G8B8A8_SINT,
	VK_FORMAT_R8G8B8A8_SRGB,
	VK_FORMAT_B8G8R8A8_UNORM,
	VK_FORMAT_B8G8R8A8_SNORM,
	VK_FORMAT_B8G8R8A8_USCALED,
	VK_FORMAT_B8G8R8A8_SSCALED,
	VK_FORMAT_B8G8R8A8_UINT,
	VK_FORMAT_B8G8R8A8_SINT,
	VK_FORMAT_B8G8R8A8_SRGB,
	VK_FORMAT_A8B8G8R8_UNORM_PACK32,
	VK_FORMAT_A8B8G8R8_SNORM_PACK32,
	VK_FORMAT_A8B8G8R8_USCALED_PACK32,
	VK_FORMAT_A8B8G8R8_SSCALED_PACK32,
	VK_FORMAT_A8B8G8R8_UINT_PACK32,
	VK_FORMAT_A8B8G8R8_SINT_PACK32,
	VK_FORMAT_A8B8G8R8_SRGB_PACK32,
	VK_FORMAT_A2R10G10B10_UNORM_PACK32,
	VK_FORMAT_A2R10G10B10_SNORM_PACK32,
	VK_FORMAT_A2R10G10B10_USCALED_PACK32,
	VK_FORMAT_A2R10G10B10_SSCALED_PACK32,
	VK_FORMAT_A2R10G10B10_UINT_PACK32,
	VK_FORMAT_A2R10G10B10_SINT_PACK32,
	VK_FORMAT_A2B10G10R10_UNORM_PACK32,
	VK_FORMAT_A2B10G10R10_SNORM_PACK32,
	VK_FORMAT_A2B10G10R10_USCALED_PACK32,
	VK_FORMAT_A2B10G10R10_SSCALED_PACK32,
	VK_FORMAT_A2B10G10R10_UINT_PACK32,
	VK_FORMAT_A2B10G10R10_SINT_PACK32,
	VK_FORMAT_R16_UNORM,
	VK_FORMAT_R16_SNORM,
	VK_FORMAT_R16_USCALED,
	VK_FORMAT_R16_SSCALED,
	VK_FORMAT_R16_UINT,
	VK_FORMAT_R16_SINT,
	VK_FORMAT_R16_SFLOAT,
	VK_FORMAT_R16G16_UNORM,
	VK_FORMAT_R16G16_SNORM,
	VK_FORMAT_R16G16_USCALED,
	VK_FORMAT_R16G16_SSCALED,
	VK_FORMAT_R16G16_UINT,
	VK_FORMAT_R16G16_SINT,
	VK_FORMAT_R16G16_SFLOAT,
	VK_FORMAT_R16G16B16_UNORM,
	VK_FORMAT_R16G16B16_SNORM,
	VK_FORMAT_R16G16B16_USCALED,
	VK_FORMAT_R16G16B16_SSCALED,
	VK_FORMAT_R16G16B16_UINT,
	VK_FORMAT_R16G16B16_SINT,
	VK_FORMAT_R16G16B16_SFLOAT,
	VK_FORMAT_R16G16B16A16_UNORM,
	VK_FORMAT_R16G16B16A16_SNORM,
	VK_FORMAT_R16G16B16A16_USCALED,
	VK_FORMAT_R16G16B16A16_SSCALED,
	VK_FORMAT_R16G16B16A16_UINT,
	VK_FORMAT_R16G16B16A16_SINT,
	VK_FORMAT_R16G16B16A16_SFLOAT,
	VK_FORMAT_R32_UINT,
	VK_FORMAT_R32_SINT,
	VK_FORMAT_R32_SFLOAT,
	VK_FORMAT_R32G32_UINT,
	VK_FORMAT_R32G32_SINT,
	VK_FORMAT_R32G32_SFLOAT,
	VK_FORMAT_R32G32B32_UINT,
	VK_FORMAT_R32G32B32_SINT,
	VK_FORMAT_R32G32B32_SFLOAT,
	VK_FORMAT_R32G32B32A32_UINT,
	VK_FORMAT_R32G32B32A32_SINT,
	VK_FORMAT_R32G32B32A32_SFLOAT,
	VK_FORMAT_R64_UINT,
	VK_FORMAT_R64_SINT,
	VK_FORMAT_R64_SFLOAT,
	VK_FORMAT_R64G64_UINT,
	VK_FORMAT_R64G64_SINT,
	VK_FORMAT_R64G64_SFLOAT,
	VK_FORMAT_R64G64B64_UINT,
	VK_FORMAT_R64G64B64_SINT,
	VK_FORMAT_R64G64B64_SFLOAT,
	VK_FORMAT_R64G64B64A64_UINT,
	VK_FORMAT_R64G64B64A64_SINT,
	VK_FORMAT_R64G64B64A64_SFLOAT,
	VK_FORMAT_B10G11R11_UFLOAT_PACK32,
	VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
	VK_FORMAT_D16_UNORM,
	VK_FORMAT_X8_D24_UNORM_PACK32,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_BC1_RGB_UNORM_BLOCK,
	VK_FORMAT_BC1_RGB_SRGB_BLOCK,
	VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	VK_FORMAT_BC2_UNORM_BLOCK,
	VK_FORMAT_BC2_SRGB_BLOCK,
	VK_FORMAT_BC3_UNORM_BLOCK,
	VK_FORMAT_BC3_SRGB_BLOCK,
	VK_FORMAT_BC4_UNORM_BLOCK,
	VK_FORMAT_BC4_SNORM_BLOCK,
	VK_FORMAT_BC5_UNORM_BLOCK,
	VK_FORMAT_BC5_SNORM_BLOCK,
	VK_FORMAT_BC6H_UFLOAT_BLOCK,
	VK_FORMAT_BC6H_SFLOAT_BLOCK,
	VK_FORMAT_BC7_UNORM_BLOCK,
	VK_FORMAT_BC7_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK,
	VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK,
	VK_FORMAT_EAC_R11_UNORM_BLOCK,
	VK_FORMAT_EAC_R11_SNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_UNORM_BLOCK,
	VK_FORMAT_EAC_R11G11_SNORM_BLOCK,
	VK_FORMAT_ASTC_4x4_UNORM_BLOCK,
	VK_FORMAT_ASTC_4x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x4_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x4_SRGB_BLOCK,
	VK_FORMAT_ASTC_5x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_5x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_6x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_6x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_8x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_8x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x5_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x5_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x6_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x6_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x8_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x8_SRGB_BLOCK,
	VK_FORMAT_ASTC_10x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_10x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x10_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x10_SRGB_BLOCK,
	VK_FORMAT_ASTC_12x12_UNORM_BLOCK,
	VK_FORMAT_ASTC_12x12_SRGB_BLOCK,
	VK_FORMAT_G8B8G8R8_422_UNORM,
	VK_FORMAT_B8G8R8G8_422_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_422_UNORM,
	VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM,
	VK_FORMAT_R10X6_UNORM_PACK16,
	VK_FORMAT_R10X6G10X6_UNORM_2PACK16,
	VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16,
	VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
	VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_R12X4_UNORM_PACK16,
	VK_FORMAT_R12X4G12X4_UNORM_2PACK16,
	VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16,
	VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
	VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G16B16G16R16_422_UNORM,
	VK_FORMAT_B16G16R16G16_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_420_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM,
	VK_FORMAT_G16_B16R16_2PLANE_422_UNORM,
	VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM,
	VK_FORMAT_G8_B8R8_2PLANE_444_UNORM,
	VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16,
	VK_FORMAT_G16_B16R16_2PLANE_444_UNORM,
	VK_FORMAT_A4R4G4B4_UNORM_PACK16,
	VK_FORMAT_A4B4G4R4_UNORM_PACK16,
	VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK,
	VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK,
	VK_FORMAT_A1B5G5R5_UNORM_PACK16,
	VK_FORMAT_A8_UNORM,
	VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG,
	VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG,
	VK_FORMAT_R8_BOOL_ARM,
	VK_FORMAT_R16G16_SFIXED5_NV,
	VK_FORMAT_R10X6_UINT_PACK16_ARM,
	VK_FORMAT_R10X6G10X6_UINT_2PACK16_ARM,
	VK_FORMAT_R10X6G10X6B10X6A10X6_UINT_4PACK16_ARM,
	VK_FORMAT_R12X4_UINT_PACK16_ARM,
	VK_FORMAT_R12X4G12X4_UINT_2PACK16_ARM,
	VK_FORMAT_R12X4G12X4B12X4A12X4_UINT_4PACK16_ARM,
	VK_FORMAT_R14X2_UINT_PACK16_ARM,
	VK_FORMAT_R14X2G14X2_UINT_2PACK16_ARM,
	VK_FORMAT_R14X2G14X2B14X2A14X2_UINT_4PACK16_ARM,
	VK_FORMAT_R14X2_UNORM_PACK16_ARM,
	VK_FORMAT_R14X2G14X2_UNORM_2PACK16_ARM,
	VK_FORMAT_R14X2G14X2B14X2A14X2_UNORM_4PACK16_ARM,
	VK_FORMAT_G14X2_B14X2R14X2_2PLANE_420_UNORM_3PACK16_ARM,
	VK_FORMAT_G14X2_B14X2R14X2_2PLANE_422_UNORM_3PACK16_ARM,
};

constexpr int vk_r3r_info_impl_vk_format_count = std::extent<decltype(vk_r3r_info_impl_vk_formats)>::value;

} // namespace

// ======================================

class VkR3rInfo::Impl
{
public:
	Impl(sys::Logger& logger, const VkR3rContext& context);
	~Impl();

	void log_validation_layers();
	void log_enabled_validation_layers();
	void log_enabled_extensions();
	void log_extensions();
	void log_surface_capabilities();
	void log_physical_devices();

private:
	static const std::size_t indentation_delta = 2;
	static constexpr const char* const colon_space = ": ";

	static const std::string& prefix;

	sys::Logger* logger_{};
	const VkR3rContext* context_{};
	std::string indentation_{};
	std::string string_{};

	template<typename T>
	void append_number(T value);
	void append_number(float value);
	template<typename T>
	void append_number_hex(T value);
	template<typename T>
	void append_number_dec_hex(T number);
	void append_api_version_property(std::uint32_t api_version, const char* caption);
	void append_bool_property(VkBool32 value, const char* caption);
	template<typename TFlag>
	void append_flag_property(TFlag flag, const char* caption, const char* (*flag_name_func)(TFlag));
	template<typename TFlagBits>
	void append_flags_property(VkFlags flags, const char* caption, const char* (*flag_name_func)(TFlagBits));
	void append_sample_count_flags_property(VkSampleCountFlags flags, const char* caption);
	void append_image_usage_flags_property(VkImageUsageFlags flags, const char* caption);
	void append_vk_format_feature_flags_property(VkFormatFeatureFlags flags, const char* caption);
	void append_surface_transform_property(VkSurfaceTransformFlagBitsKHR flag, const char* caption);
	void append_surface_transform_flags_property(VkSurfaceTransformFlagsKHR flags, const char* caption);
	void append_composite_alphas_property(VkCompositeAlphaFlagsKHR flags, const char* caption);
	void append_extent_2d_property(const VkExtent2D& value, const char* caption);
	void append_vk_format(VkFormat vk_format);
	void append_vk_color_space_khr(VkColorSpaceKHR vk_color_space_khr);
	void append_vk_present_mode_khr(VkPresentModeKHR vk_present_mode_khr);
	template<typename T>
	void append_number_property(T number, const char* name);
	template<typename T, std::size_t N>
	void append_number_array_property(const T (&array)[N], const char* name);
	template<typename T>
	void append_number_dec_hex_property(T number, const char* name);
	void append_vk_format_property(VkFormat vk_format, const char* name);
	void append_vk_color_space_khr_property(VkColorSpaceKHR vk_color_space_khr, const char* name);

	void clear();
	void append_prefix();
	void clear_indentation();
	void append_indentation();
	void increase_indentation();
	void decrease_indentation();
	void append(char ch);
	void append(const char* string);
	void append(const std::string& string);
	void append_newline();
	void append_newline_if_not_empty();
	void append_line(char ch);
	void append_line(const char* string);
	void append_line(const std::string& string);
	void append_api_version(std::uint32_t api_version);
	void append_uuid(const std::uint8_t (&uuid_bytes)[uuid_value_size]);

	void append_physical_device_vendor_id_property(std::uint32_t vk_vendor_id, const char* name);
	void append_physical_device_type(VkPhysicalDeviceType type);
	void append_physical_device_limits(const VkPhysicalDeviceLimits& limits);
	void append_physical_device_sparse_properties(const VkPhysicalDeviceSparseProperties& properties);
	void append_physical_device_properties(const VkPhysicalDeviceProperties& properties);
	void append_physical_device_features(const VkPhysicalDeviceFeatures& features);
	void append_physical_device_features(VkPhysicalDevice physical_device);
	void append_memory_property_flags(VkMemoryPropertyFlags flags, const char* caption);
	void append_physical_device_memory_type(const VkMemoryType& type);
	void append_physical_device_memory_types(const VkPhysicalDeviceMemoryProperties& properties);
	void append_memory_heap_size(const VkMemoryHeap& heap);
	void append_memory_heap_flags(VkMemoryHeapFlags flags, const char* caption);
	void append_memory_heap(const VkMemoryHeap& heap);
	void append_physical_device_memory_heaps(const VkPhysicalDeviceMemoryProperties& properties);
	void append_physical_device_memory_properties(const VkPhysicalDeviceMemoryProperties& properties);
	void append_physical_device_memory_properties(VkPhysicalDevice physical_device);
	void append_physical_device_format_properties(VkFormat vk_format, const VkFormatProperties& vk_format_properties);
	void append_physical_device_formats_properties(VkPhysicalDevice physical_device);
	void append_physical_device_surface_format(const VkSurfaceFormatKHR& format);
	void append_physical_device_surface_formats(VkPhysicalDevice physical_device);
	void append_physical_device_surface_present_modes(VkPhysicalDevice physical_device);
	void append_physical_device(VkPhysicalDevice physical_device);
};

// --------------------------------------

const std::string& VkR3rInfo::Impl::prefix = "[VK] ";

// --------------------------------------

template<typename T>
void VkR3rInfo::Impl::append_number(T value)
{
	constexpr int buffer_size = 32;
	char char_buffer[buffer_size];
	char* const char_end_iter = to_chars(value, char_buffer, &char_buffer[buffer_size]);
	const std::size_t char_count = static_cast<std::size_t>(char_end_iter - char_buffer);
	string_.append(char_buffer, char_count);
}

void VkR3rInfo::Impl::append_number(float value)
{
	constexpr std::size_t max_buffer_size = 64;
	char char_buffer[max_buffer_size];
	const int written_count = snprintf(char_buffer, max_buffer_size, "%f", value);
	string_.append(char_buffer, static_cast<std::size_t>(written_count));
}

template<typename T>
void VkR3rInfo::Impl::append_number_hex(T value)
{
	constexpr int buffer_size = 32;
	char char_buffer[buffer_size];
	char* const char_end_iter = to_chars(value, char_buffer, &char_buffer[buffer_size], 16);
	const std::size_t char_count = static_cast<std::size_t>(char_end_iter - char_buffer);
	ascii::to_upper(char_buffer, char_end_iter);
	string_.append(char_buffer, char_count);
}

template<typename T>
void VkR3rInfo::Impl::append_number_dec_hex(T value)
{
	append_number(value);
	append(" (0x");
	append_number_hex(value);
	append(')');
}

void VkR3rInfo::Impl::append_api_version_property(std::uint32_t api_version, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append_api_version(api_version);
	append_newline();
}

void VkR3rInfo::Impl::append_bool_property(VkBool32 value, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append(value != 0 ? "true" : "false");
	append_newline();
}

template<typename TFlag>
void VkR3rInfo::Impl::append_flag_property(TFlag flag, const char* caption, const char* (*flag_name_func)(TFlag))
{
	using FlagInt = std::conditional_t<std::is_enum<TFlag>::value, std::underlying_type_t<TFlag>, TFlag>;
	const char* const flag_name = flag_name_func(flag);
	append_indentation();
	append(caption);
	append(": ");
	if (flag_name != nullptr)
	{
		append_line(flag_name);
	}
	else
	{
		append("0x");
		append_number_hex(static_cast<FlagInt>(flag));
		append_newline();
	}
}

template<typename TFlagBits>
void VkR3rInfo::Impl::append_flags_property(VkFlags flags, const char* caption, const char* (*flag_name_func)(TFlagBits))
{
	if (caption != nullptr)
	{
		append_indentation();
		append(caption);
		append(": (0x");
		append_number_hex(flags);
		append_line(")");
	}
	// "Only the low-order 31 bits (bit positions zero through 30) are available for use as flag bits."
	flags &= 0x7FFFFFFFU;
	increase_indentation();
	int bit_index = 0;
	for (; flags != 0; flags >>= 1)
	{
		const VkFlags flag = VkFlags{1} << bit_index;
		if ((flags & 1) != 0)
		{
			append_indentation();
			const char* flag_name = flag_name_func(static_cast<TFlagBits>(flag));
			if (flag_name != nullptr)
			{
				append_line(flag_name);
			}
			else
			{
				append("0x");
				append_number_hex(flag);
				append_newline();
			}
		}
		++bit_index;
	}
	decrease_indentation();
}

void VkR3rInfo::Impl::append_sample_count_flags_property(VkSampleCountFlags flags, const char* caption)
{
	append_flags_property<VkSampleCountFlagBits>(flags, caption, &VkR3rEnumStrings::get_VkSampleCountFlagBits);
}

void VkR3rInfo::Impl::append_image_usage_flags_property(VkImageUsageFlags flags, const char* caption)
{
	append_flags_property<VkImageUsageFlagBits>(flags, caption, &VkR3rEnumStrings::get_VkImageUsageFlagBits);
}

void VkR3rInfo::Impl::append_vk_format_feature_flags_property(VkFormatFeatureFlags flags, const char* caption)
{
	append_flags_property<VkFormatFeatureFlagBits>(flags, caption, &VkR3rEnumStrings::get_VkFormatFeatureFlagBits);
}

void VkR3rInfo::Impl::append_surface_transform_property(VkSurfaceTransformFlagBitsKHR flag, const char* caption)
{
	append_flag_property<VkSurfaceTransformFlagBitsKHR>(flag, caption, &VkR3rEnumStrings::get_VkSurfaceTransformFlagBitsKHR);
}

void VkR3rInfo::Impl::append_surface_transform_flags_property(VkSurfaceTransformFlagsKHR flags, const char* caption)
{
	append_flags_property<VkSurfaceTransformFlagBitsKHR>(flags, caption, &VkR3rEnumStrings::get_VkSurfaceTransformFlagBitsKHR);
}

void VkR3rInfo::Impl::append_composite_alphas_property(VkCompositeAlphaFlagsKHR flags, const char* caption)
{
	append_flags_property<VkCompositeAlphaFlagBitsKHR>(flags, caption, &VkR3rEnumStrings::get_VkCompositeAlphaFlagBitsKHR);
}

void VkR3rInfo::Impl::append_extent_2d_property(const VkExtent2D& value, const char* name)
{
	append_indentation();
	append(name);
	append(": [");
	append_number(value.width);
	append(", ");
	append_number(value.height);
	append_line(']');
}

void VkR3rInfo::Impl::append_vk_format(VkFormat vk_format)
{
	const char* const vk_format_string = VkR3rEnumStrings::get_VkFormat(vk_format);
	if (vk_format_string != nullptr)
	{
		append(vk_format_string);
	}
	else
	{
		append_number_dec_hex(static_cast<int>(vk_format));
	}
}

void VkR3rInfo::Impl::append_vk_color_space_khr(VkColorSpaceKHR vk_color_space_khr)
{
	const char* const vk_color_space_khr_string = VkR3rEnumStrings::get_VkColorSpaceKHR(vk_color_space_khr);
	if (vk_color_space_khr_string != nullptr)
	{
		append(vk_color_space_khr_string);
	}
	else
	{
		append_number_dec_hex(static_cast<int>(vk_color_space_khr));
	}
}

void VkR3rInfo::Impl::append_vk_present_mode_khr(VkPresentModeKHR vk_present_mode_khr)
{
	const char* const vk_present_mode_khr_string = VkR3rEnumStrings::get_VkPresentModeKHR(vk_present_mode_khr);
	if (vk_present_mode_khr_string != nullptr)
	{
		append(vk_present_mode_khr_string);
	}
	else
	{
		append_number_dec_hex(static_cast<int>(vk_present_mode_khr));
	}
}

template<typename T>
void VkR3rInfo::Impl::append_number_property(T number, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append_number(number);
	append_newline();
}

template<typename T, std::size_t N>
void VkR3rInfo::Impl::append_number_array_property(const T (&array)[N], const char* name)
{
	append_indentation();
	append(name);
	append(": [");
	for (std::size_t i = 0; i < N; ++i)
	{
		if (i > 0)
		{
			append(", ");
		}
		append_number(array[i]);
	}
	append(']');
	append_newline();
}

template<typename T>
void VkR3rInfo::Impl::append_number_dec_hex_property(T number, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append_number_dec_hex(number);
	append_newline();
}

void VkR3rInfo::Impl::append_vk_format_property(VkFormat vk_format, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append_vk_format(vk_format);
	append_newline();
}

void VkR3rInfo::Impl::append_vk_color_space_khr_property(VkColorSpaceKHR vk_color_space_khr, const char* name)
{
	append_indentation();
	append(name);
	append(": ");
	append_vk_color_space_khr(vk_color_space_khr);
	append_newline();
}

// ==========================================================================

VkR3rInfo::Impl::Impl(sys::Logger& logger, const VkR3rContext& context)
{
	logger_ = &logger;
	context_ = &context;
	string_.reserve(1048576);
	indentation_.resize(prefix.size(), ' ');
}

VkR3rInfo::Impl::~Impl() = default;

void VkR3rInfo::Impl::log_validation_layers()
{
	clear();
	append_prefix();
	append_line("Validation layers:");
	const auto layers = vk_r3r_extract_array(context_->vkEnumerateInstanceLayerProperties);
	increase_indentation();
	bool is_first = true;
	for (const VkLayerProperties& layer : layers)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			append_newline();
		}
		append_indentation();
		append(layer.layerName);
	}
	logger_->log_information(string_.c_str());
}

void VkR3rInfo::Impl::log_enabled_validation_layers()
{
	clear();
	append_prefix();
	append_line("Enabled validation layers:");
	increase_indentation();
	bool is_first = true;
	for (const char* layer_name : context_->enabled_layers)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			append_newline();
		}
		append_indentation();
		append(layer_name);
	}
	logger_->log_information(string_.c_str());
}

void VkR3rInfo::Impl::log_enabled_extensions()
{
	clear();
	append_prefix();
	append_line("Enabled extensions:");
	increase_indentation();
	bool is_first = true;
	for (const char* extensions_name : context_->enabled_extensions)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			append_newline();
		}

		append_indentation();
		append(extensions_name);
	}
	logger_->log_information(string_.c_str());
}

void VkR3rInfo::Impl::log_extensions()
{
	clear();
	append_prefix();
	append_line("Extensions:");
	const auto extensions = vk_r3r_extract_array(context_->vkEnumerateInstanceExtensionProperties, nullptr);
	increase_indentation();
	bool is_first = true;
	for (const VkExtensionProperties& extension : extensions)
	{
		if (is_first)
		{
			is_first = false;
		}
		else
		{
			append_newline();
		}
		append_indentation();
		append(extension.extensionName);
	}
	logger_->log_information(string_.c_str());
}

void VkR3rInfo::Impl::log_surface_capabilities()
{
#define BSTONE_APPEND_NUM(x) append_number_property(surface_capabilities.x, #x)
#define BSTONE_APPEND_EXT2(x) append_extent_2d_property(surface_capabilities.x, #x)
#define BSTONE_APPEND_TRNS(x) append_surface_transform_flags_property(surface_capabilities.x, #x)
#define BSTONE_APPEND_TRAN(x) append_surface_transform_property(surface_capabilities.x, #x)
#define BSTONE_APPEND_CALP(x) append_composite_alphas_property(surface_capabilities.x, #x)
#define BSTONE_APPEND_IMGU(x) append_image_usage_flags_property(surface_capabilities.x, #x)
	const VkSurfaceCapabilitiesKHR& surface_capabilities = context_->surface_capabilities;
	clear();
	append_prefix();
	append_line("Surface capabilities:");
	increase_indentation();
	BSTONE_APPEND_NUM(minImageCount);
	BSTONE_APPEND_NUM(maxImageCount);
	BSTONE_APPEND_EXT2(currentExtent);
	BSTONE_APPEND_EXT2(minImageExtent);
	BSTONE_APPEND_EXT2(maxImageExtent);
	BSTONE_APPEND_NUM(maxImageArrayLayers);
    BSTONE_APPEND_TRNS(supportedTransforms);
	BSTONE_APPEND_TRAN(currentTransform);
    BSTONE_APPEND_CALP(supportedCompositeAlpha);
    BSTONE_APPEND_IMGU(supportedUsageFlags);
	decrease_indentation();
	logger_->log_information(string_.c_str());
#undef BSTONE_APPEND_NUM
#undef BSTONE_APPEND_EXT2
#undef BSTONE_APPEND_TRNS
#undef BSTONE_APPEND_TRAN
#undef BSTONE_APPEND_CALP
#undef BSTONE_APPEND_IMGU
}

void VkR3rInfo::Impl::log_physical_devices()
{
	clear();
	append_prefix();
	append_line("Physical devices:");
	const auto physical_devices = vk_r3r_extract_array(
		context_->vkEnumeratePhysicalDevices, context_->instance.get());
	std::size_t device_number = 1;
	for (const VkPhysicalDevice physical_device : physical_devices)
	{
		increase_indentation();
		append_indentation();
		append_number(device_number);
		append_line(')');
		append_physical_device(physical_device);
		decrease_indentation();
		++device_number;
	}
	logger_->log_information(string_.c_str());
}

void VkR3rInfo::Impl::clear()
{
	string_.clear();
	clear_indentation();
}

void VkR3rInfo::Impl::append_prefix()
{
	string_ += prefix;
}

void VkR3rInfo::Impl::clear_indentation()
{
	indentation_.clear();
	indentation_.resize(prefix.size(), ' ');
}

void VkR3rInfo::Impl::append_indentation()
{
	string_ += indentation_;
}

void VkR3rInfo::Impl::increase_indentation()
{
	indentation_.resize(indentation_.size() + indentation_delta, ' ');
}

void VkR3rInfo::Impl::decrease_indentation()
{
	BSTONE_ASSERT(indentation_.size() >= prefix.size() + indentation_delta);
	indentation_.resize(indentation_.size() - indentation_delta, ' ');
}

void VkR3rInfo::Impl::append(char ch)
{
	string_ += ch;
}

void VkR3rInfo::Impl::append(const char* string)
{
	string_ += string;
}

void VkR3rInfo::Impl::append(const std::string& string)
{
	string_ += string;
}

void VkR3rInfo::Impl::append_line(char ch)
{
	append(ch);
	append_newline();
}

void VkR3rInfo::Impl::append_line(const char* string)
{
	append(string);
	append_newline();
}

void VkR3rInfo::Impl::append_line(const std::string& string)
{
	append(string);
	append_newline();
}

void VkR3rInfo::Impl::append_newline()
{
	append('\n');
}

void VkR3rInfo::Impl::append_newline_if_not_empty()
{
	if (!string_.empty())
	{
		append_newline();
	}
}

void VkR3rInfo::Impl::append_api_version(std::uint32_t api_version)
{
	const std::uint32_t version_variant = VK_API_VERSION_VARIANT(api_version);
	const std::uint32_t version_major = VK_API_VERSION_MAJOR(api_version);
	const std::uint32_t version_minor = VK_API_VERSION_MINOR(api_version);
	const std::uint32_t version_patch = VK_API_VERSION_PATCH(api_version);
	if (version_variant != 0)
	{
		append_number(version_variant);
		append('.');
	}
	append_number(version_major);
	append('.');
	append_number(version_minor);
	append('.');
	append_number(version_patch);
}

void VkR3rInfo::Impl::append_uuid(const std::uint8_t (&uuid_bytes)[uuid_value_size])
{
	constexpr int max_uuid_chars = 40;
	char uuid_chars[max_uuid_chars];
	const Uuid uuid{uuid_bytes};
	char* const end_iter = uuid.to_chars(uuid_chars, &uuid_chars[max_uuid_chars]);
	end_iter[0] = '\0';
	append(uuid_chars);
}

void VkR3rInfo::Impl::append_physical_device_vendor_id_property(std::uint32_t vk_vendor_id, const char* name)
{
	const char* const id_string = VkR3rEnumStrings::get_VkVendorId(static_cast<VkVendorId>(vk_vendor_id));
	append_indentation();
	append(name);
	append(": ");
	if (id_string != nullptr)
	{
		append(id_string);
		append(" (");
	}
	append_number_dec_hex(vk_vendor_id);
	if (id_string != nullptr)
	{
		append(')');
	}
	append_newline();
}

void VkR3rInfo::Impl::append_physical_device_type(VkPhysicalDeviceType type)
{
	const char* const type_string = VkR3rEnumStrings::get_VkPhysicalDeviceType(type);
	if (type_string != nullptr)
	{
		append(type_string);
		append(" (");
	}
	append_number_dec_hex(static_cast<std::underlying_type_t<decltype(type)>>(type));
	if (type_string != nullptr)
	{
		append(')');
	}
}

void VkR3rInfo::Impl::append_physical_device_limits(const VkPhysicalDeviceLimits& limits)
{
#define BSTONE_APPEND_BOOL(x) append_bool_property(limits.x, #x)
#define BSTONE_APPEND_NUM(x) append_number_property(limits.x, #x)
#define BSTONE_APPEND_NUMN(x) append_number_array_property(limits.x, #x)
#define BSTONE_APPEND_SAMP(x) append_sample_count_flags_property(limits.x, #x)
	BSTONE_APPEND_NUM(maxImageDimension1D);
	BSTONE_APPEND_NUM(maxImageDimension2D);
	BSTONE_APPEND_NUM(maxImageDimension3D);
	BSTONE_APPEND_NUM(maxImageDimensionCube);
	BSTONE_APPEND_NUM(maxImageArrayLayers);
	BSTONE_APPEND_NUM(maxTexelBufferElements);
	BSTONE_APPEND_NUM(maxUniformBufferRange);
	BSTONE_APPEND_NUM(maxStorageBufferRange);
	BSTONE_APPEND_NUM(maxPushConstantsSize);
	BSTONE_APPEND_NUM(maxMemoryAllocationCount);
	BSTONE_APPEND_NUM(maxSamplerAllocationCount);
	BSTONE_APPEND_NUM(bufferImageGranularity);
	BSTONE_APPEND_NUM(sparseAddressSpaceSize);
	BSTONE_APPEND_NUM(maxBoundDescriptorSets);
	BSTONE_APPEND_NUM(maxPerStageDescriptorSamplers);
	BSTONE_APPEND_NUM(maxPerStageDescriptorUniformBuffers);
	BSTONE_APPEND_NUM(maxPerStageDescriptorStorageBuffers);
	BSTONE_APPEND_NUM(maxPerStageDescriptorSampledImages);
	BSTONE_APPEND_NUM(maxPerStageDescriptorStorageImages);
	BSTONE_APPEND_NUM(maxPerStageDescriptorInputAttachments);
	BSTONE_APPEND_NUM(maxPerStageResources);
	BSTONE_APPEND_NUM(maxDescriptorSetSamplers);
	BSTONE_APPEND_NUM(maxDescriptorSetUniformBuffers);
	BSTONE_APPEND_NUM(maxDescriptorSetUniformBuffersDynamic);
	BSTONE_APPEND_NUM(maxDescriptorSetStorageBuffers);
	BSTONE_APPEND_NUM(maxDescriptorSetStorageBuffersDynamic);
	BSTONE_APPEND_NUM(maxDescriptorSetSampledImages);
	BSTONE_APPEND_NUM(maxDescriptorSetStorageImages);
	BSTONE_APPEND_NUM(maxDescriptorSetInputAttachments);
	BSTONE_APPEND_NUM(maxVertexInputAttributes);
	BSTONE_APPEND_NUM(maxVertexInputBindings);
	BSTONE_APPEND_NUM(maxVertexInputAttributeOffset);
	BSTONE_APPEND_NUM(maxVertexInputBindingStride);
	BSTONE_APPEND_NUM(maxVertexOutputComponents);
	BSTONE_APPEND_NUM(maxTessellationGenerationLevel);
	BSTONE_APPEND_NUM(maxTessellationPatchSize);
	BSTONE_APPEND_NUM(maxTessellationControlPerVertexInputComponents);
	BSTONE_APPEND_NUM(maxTessellationControlPerVertexOutputComponents);
	BSTONE_APPEND_NUM(maxTessellationControlPerPatchOutputComponents);
	BSTONE_APPEND_NUM(maxTessellationControlTotalOutputComponents);
	BSTONE_APPEND_NUM(maxTessellationEvaluationInputComponents);
	BSTONE_APPEND_NUM(maxTessellationEvaluationOutputComponents);
	BSTONE_APPEND_NUM(maxGeometryShaderInvocations);
	BSTONE_APPEND_NUM(maxGeometryInputComponents);
	BSTONE_APPEND_NUM(maxGeometryOutputComponents);
	BSTONE_APPEND_NUM(maxGeometryOutputVertices);
	BSTONE_APPEND_NUM(maxGeometryTotalOutputComponents);
	BSTONE_APPEND_NUM(maxFragmentInputComponents);
	BSTONE_APPEND_NUM(maxFragmentOutputAttachments);
	BSTONE_APPEND_NUM(maxFragmentDualSrcAttachments);
	BSTONE_APPEND_NUM(maxFragmentCombinedOutputResources);
	BSTONE_APPEND_NUM(maxComputeSharedMemorySize);
	BSTONE_APPEND_NUMN(maxComputeWorkGroupCount);
	BSTONE_APPEND_NUM(maxComputeWorkGroupInvocations);
	BSTONE_APPEND_NUMN(maxComputeWorkGroupSize);
	BSTONE_APPEND_NUM(subPixelPrecisionBits);
	BSTONE_APPEND_NUM(subTexelPrecisionBits);
	BSTONE_APPEND_NUM(mipmapPrecisionBits);
	BSTONE_APPEND_NUM(maxDrawIndexedIndexValue);
	BSTONE_APPEND_NUM(maxDrawIndirectCount);
	BSTONE_APPEND_NUM(maxSamplerLodBias);
	BSTONE_APPEND_NUM(maxSamplerAnisotropy);
	BSTONE_APPEND_NUM(maxViewports);
	BSTONE_APPEND_NUMN(maxViewportDimensions);
	BSTONE_APPEND_NUMN(viewportBoundsRange);
	BSTONE_APPEND_NUM(viewportSubPixelBits);
	BSTONE_APPEND_NUM(minMemoryMapAlignment);
	BSTONE_APPEND_NUM(minTexelBufferOffsetAlignment);
	BSTONE_APPEND_NUM(minUniformBufferOffsetAlignment);
	BSTONE_APPEND_NUM(minStorageBufferOffsetAlignment);
	BSTONE_APPEND_NUM(minTexelOffset);
	BSTONE_APPEND_NUM(maxTexelOffset);
	BSTONE_APPEND_NUM(minTexelGatherOffset);
	BSTONE_APPEND_NUM(maxTexelGatherOffset);
	BSTONE_APPEND_NUM(minInterpolationOffset);
	BSTONE_APPEND_NUM(maxInterpolationOffset);
	BSTONE_APPEND_NUM(subPixelInterpolationOffsetBits);
	BSTONE_APPEND_NUM(maxFramebufferWidth);
	BSTONE_APPEND_NUM(maxFramebufferHeight);
	BSTONE_APPEND_NUM(maxFramebufferLayers);
	BSTONE_APPEND_SAMP(framebufferColorSampleCounts);
	BSTONE_APPEND_SAMP(framebufferDepthSampleCounts);
	BSTONE_APPEND_SAMP(framebufferStencilSampleCounts);
	BSTONE_APPEND_SAMP(framebufferNoAttachmentsSampleCounts);
	BSTONE_APPEND_NUM(maxColorAttachments);
	BSTONE_APPEND_SAMP(sampledImageColorSampleCounts);
	BSTONE_APPEND_SAMP(sampledImageIntegerSampleCounts);
	BSTONE_APPEND_SAMP(sampledImageDepthSampleCounts);
	BSTONE_APPEND_SAMP(sampledImageStencilSampleCounts);
	BSTONE_APPEND_SAMP(storageImageSampleCounts);
	BSTONE_APPEND_NUM(maxSampleMaskWords);
	BSTONE_APPEND_BOOL(timestampComputeAndGraphics);
	BSTONE_APPEND_NUM(timestampPeriod);
	BSTONE_APPEND_NUM(maxClipDistances);
	BSTONE_APPEND_NUM(maxCullDistances);
	BSTONE_APPEND_NUM(maxCombinedClipAndCullDistances);
	BSTONE_APPEND_NUM(discreteQueuePriorities);
	BSTONE_APPEND_NUMN(pointSizeRange);
	BSTONE_APPEND_NUMN(lineWidthRange);
	BSTONE_APPEND_NUM(pointSizeGranularity);
	BSTONE_APPEND_NUM(lineWidthGranularity);
	BSTONE_APPEND_BOOL(strictLines);
	BSTONE_APPEND_BOOL(standardSampleLocations);
	BSTONE_APPEND_NUM(optimalBufferCopyOffsetAlignment);
	BSTONE_APPEND_NUM(optimalBufferCopyRowPitchAlignment);
	BSTONE_APPEND_NUM(nonCoherentAtomSize);
#undef BSTONE_APPEND_BOOL
#undef BSTONE_APPEND_NUM
#undef BSTONE_APPEND_NUMN
#undef BSTONE_APPEND_SAMP
}

void VkR3rInfo::Impl::append_physical_device_sparse_properties(const VkPhysicalDeviceSparseProperties& properties)
{
#define BSTONE_APPEND_BOOL(x) append_bool_property(properties.x, #x)
	BSTONE_APPEND_BOOL(residencyStandard2DBlockShape);
	BSTONE_APPEND_BOOL(residencyStandard2DMultisampleBlockShape);
	BSTONE_APPEND_BOOL(residencyStandard3DBlockShape);
	BSTONE_APPEND_BOOL(residencyAlignedMipSize);
	BSTONE_APPEND_BOOL(residencyNonResidentStrict);
#undef BSTONE_APPEND_BOOL
}

void VkR3rInfo::Impl::append_physical_device_properties(const VkPhysicalDeviceProperties& properties)
{
	append_api_version_property(properties.apiVersion, "apiVersion");
	append_number_dec_hex_property(properties.driverVersion, "driverVersion");
	append_physical_device_vendor_id_property(properties.vendorID, "vendorID");
	append_number_dec_hex_property(properties.deviceID, "deviceID");
	// deviceType
	append_indentation();
	append("deviceType");
	append(": ");
	append_physical_device_type(properties.deviceType);
	append_newline();
	// deviceName
	append_indentation();
	append("deviceName");
	append(": ");
	append(properties.deviceName);
	append_newline();
	// pipelineCacheUUID
	append_indentation();
	append("pipelineCacheUUID");
	append(": ");
	append_uuid(properties.pipelineCacheUUID);
	append_newline();
	// limits
	append_indentation();
	append_line("Limits:");
	increase_indentation();
	append_physical_device_limits(properties.limits);
	decrease_indentation();
	// sparse properties
	append_indentation();
	append_line("Sparse properties:");
	increase_indentation();
	append_physical_device_sparse_properties(properties.sparseProperties);
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device_features(const VkPhysicalDeviceFeatures& features)
{
#define BSTONE_APPEND_BOOL(x) append_bool_property(features.x, #x)
	BSTONE_APPEND_BOOL(robustBufferAccess);
	BSTONE_APPEND_BOOL(fullDrawIndexUint32);
	BSTONE_APPEND_BOOL(imageCubeArray);
	BSTONE_APPEND_BOOL(independentBlend);
	BSTONE_APPEND_BOOL(geometryShader);
	BSTONE_APPEND_BOOL(tessellationShader);
	BSTONE_APPEND_BOOL(sampleRateShading);
	BSTONE_APPEND_BOOL(dualSrcBlend);
	BSTONE_APPEND_BOOL(logicOp);
	BSTONE_APPEND_BOOL(multiDrawIndirect);
	BSTONE_APPEND_BOOL(drawIndirectFirstInstance);
	BSTONE_APPEND_BOOL(depthClamp);
	BSTONE_APPEND_BOOL(depthBiasClamp);
	BSTONE_APPEND_BOOL(fillModeNonSolid);
	BSTONE_APPEND_BOOL(depthBounds);
	BSTONE_APPEND_BOOL(wideLines);
	BSTONE_APPEND_BOOL(largePoints);
	BSTONE_APPEND_BOOL(alphaToOne);
	BSTONE_APPEND_BOOL(multiViewport);
	BSTONE_APPEND_BOOL(samplerAnisotropy);
	BSTONE_APPEND_BOOL(textureCompressionETC2);
	BSTONE_APPEND_BOOL(textureCompressionASTC_LDR);
	BSTONE_APPEND_BOOL(textureCompressionBC);
	BSTONE_APPEND_BOOL(occlusionQueryPrecise);
	BSTONE_APPEND_BOOL(pipelineStatisticsQuery);
	BSTONE_APPEND_BOOL(vertexPipelineStoresAndAtomics);
	BSTONE_APPEND_BOOL(fragmentStoresAndAtomics);
	BSTONE_APPEND_BOOL(shaderTessellationAndGeometryPointSize);
	BSTONE_APPEND_BOOL(shaderImageGatherExtended);
	BSTONE_APPEND_BOOL(shaderStorageImageExtendedFormats);
	BSTONE_APPEND_BOOL(shaderStorageImageMultisample);
	BSTONE_APPEND_BOOL(shaderStorageImageReadWithoutFormat);
	BSTONE_APPEND_BOOL(shaderStorageImageWriteWithoutFormat);
	BSTONE_APPEND_BOOL(shaderUniformBufferArrayDynamicIndexing);
	BSTONE_APPEND_BOOL(shaderSampledImageArrayDynamicIndexing);
	BSTONE_APPEND_BOOL(shaderStorageBufferArrayDynamicIndexing);
	BSTONE_APPEND_BOOL(shaderStorageImageArrayDynamicIndexing);
	BSTONE_APPEND_BOOL(shaderClipDistance);
	BSTONE_APPEND_BOOL(shaderCullDistance);
	BSTONE_APPEND_BOOL(shaderFloat64);
	BSTONE_APPEND_BOOL(shaderInt64);
	BSTONE_APPEND_BOOL(shaderInt16);
	BSTONE_APPEND_BOOL(shaderResourceResidency);
	BSTONE_APPEND_BOOL(shaderResourceMinLod);
	BSTONE_APPEND_BOOL(sparseBinding);
	BSTONE_APPEND_BOOL(sparseResidencyBuffer);
	BSTONE_APPEND_BOOL(sparseResidencyImage2D);
	BSTONE_APPEND_BOOL(sparseResidencyImage3D);
	BSTONE_APPEND_BOOL(sparseResidency2Samples);
	BSTONE_APPEND_BOOL(sparseResidency4Samples);
	BSTONE_APPEND_BOOL(sparseResidency8Samples);
	BSTONE_APPEND_BOOL(sparseResidency16Samples);
	BSTONE_APPEND_BOOL(sparseResidencyAliased);
	BSTONE_APPEND_BOOL(variableMultisampleRate);
	BSTONE_APPEND_BOOL(inheritedQueries);
#undef BSTONE_APPEND_BOOL
}

void VkR3rInfo::Impl::append_physical_device_features(VkPhysicalDevice physical_device)
{
	append_indentation();
	append_line("Features:");
	increase_indentation();
	VkPhysicalDeviceFeatures vk_physical_device_features{};
	context_->vkGetPhysicalDeviceFeatures(physical_device, &vk_physical_device_features);
	append_physical_device_features(vk_physical_device_features);
	decrease_indentation();
}

void VkR3rInfo::Impl::append_memory_property_flags(VkMemoryPropertyFlags flags, const char* caption)
{
	append_flags_property<VkMemoryPropertyFlagBits>(flags, caption, &VkR3rEnumStrings::get_VkMemoryPropertyFlagBits);
}

void VkR3rInfo::Impl::append_physical_device_memory_type(const VkMemoryType& type)
{
#define BSTONE_APPEND_MEMF(x) append_memory_property_flags(type.x, #x)
	increase_indentation();
	BSTONE_APPEND_MEMF(propertyFlags);
	append_number_property(type.heapIndex, "heapIndex");
	decrease_indentation();
#undef BSTONE_APPEND_MEMF
}

void VkR3rInfo::Impl::append_physical_device_memory_types(const VkPhysicalDeviceMemoryProperties& properties)
{
	increase_indentation();
	append_indentation();
	append_line("Types:");
	increase_indentation();
	for (std::uint32_t i_type = 0; i_type < properties.memoryTypeCount; ++i_type)
	{
		increase_indentation();
		append_indentation();
		append_number(i_type + 1);
		append_line(')');
		append_physical_device_memory_type(properties.memoryTypes[i_type]);
		decrease_indentation();
	}
	decrease_indentation();
	decrease_indentation();
}

void VkR3rInfo::Impl::append_memory_heap_size(const VkMemoryHeap& heap)
{
	append_number_property(heap.size, "size");
}

void VkR3rInfo::Impl::append_memory_heap_flags(VkMemoryHeapFlags flags, const char* caption)
{
	append_flags_property<VkMemoryHeapFlagBits>(flags, caption, &VkR3rEnumStrings::get_VkMemoryHeapFlagBits);
}

void VkR3rInfo::Impl::append_memory_heap(const VkMemoryHeap& heap)
{
#define BSTONE_APPEND_FLGS(x) append_memory_heap_flags(heap.x, #x)
	increase_indentation();
	append_memory_heap_size(heap);
	BSTONE_APPEND_FLGS(flags);
	decrease_indentation();
#undef BSTONE_APPEND_FLGS
}

void VkR3rInfo::Impl::append_physical_device_memory_heaps(const VkPhysicalDeviceMemoryProperties& properties)
{
	increase_indentation();
	append_indentation();
	append_line("Heaps:");
	increase_indentation();
	for (std::uint32_t i_heap = 0; i_heap < properties.memoryHeapCount; ++i_heap)
	{
		increase_indentation();
		append_indentation();
		append_number(i_heap + 1);
		append_line(')');
		append_memory_heap(properties.memoryHeaps[i_heap]);
		decrease_indentation();
	}
	decrease_indentation();
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device_memory_properties(const VkPhysicalDeviceMemoryProperties& properties)
{
	append_physical_device_memory_types(properties);
	append_physical_device_memory_heaps(properties);
}

void VkR3rInfo::Impl::append_physical_device_memory_properties(VkPhysicalDevice physical_device)
{
	append_indentation();
	append_line("Memory properties:");
	increase_indentation();
	VkPhysicalDeviceMemoryProperties vk_physical_device_memory_properties{};
	context_->vkGetPhysicalDeviceMemoryProperties(physical_device, &vk_physical_device_memory_properties);
	append_physical_device_memory_properties(vk_physical_device_memory_properties);
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device_format_properties(VkFormat vk_format, const VkFormatProperties& vk_format_properties)
{
#define BSTONE_APPEND_FMFF(x) append_vk_format_feature_flags_property(vk_format_properties.x, #x)
	append_indentation();
	append_vk_format(vk_format);
	append_line(':');
	increase_indentation();
	BSTONE_APPEND_FMFF(linearTilingFeatures);
	decrease_indentation();
	increase_indentation();
	BSTONE_APPEND_FMFF(optimalTilingFeatures);
	decrease_indentation();
	increase_indentation();
	BSTONE_APPEND_FMFF(bufferFeatures);
	decrease_indentation();
#undef BSTONE_APPEND_FMFF
}

void VkR3rInfo::Impl::append_physical_device_formats_properties(VkPhysicalDevice physical_device)
{
	append_indentation();
	append_line("Format properties:");
	increase_indentation();
	VkFormatProperties vk_format_properties;
	for (const VkFormat& vk_format : vk_r3r_info_impl_vk_formats)
	{
		context_->vkGetPhysicalDeviceFormatProperties(physical_device, vk_format, &vk_format_properties);
		if (vk_format_properties.linearTilingFeatures != VkFormatFeatureFlags{} ||
			vk_format_properties.optimalTilingFeatures != VkFormatFeatureFlags{} ||
			vk_format_properties.bufferFeatures != VkFormatFeatureFlags{})
		{
			append_physical_device_format_properties(vk_format, vk_format_properties);
		}
	}
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device_surface_format(const VkSurfaceFormatKHR& format)
{
#define BSTONE_APPEND_FMT(x) append_vk_format_property(format.x, #x)
#define BSTONE_APPEND_CSPC(x) append_vk_color_space_khr_property(format.x, #x)
	BSTONE_APPEND_FMT(format);
	BSTONE_APPEND_CSPC(colorSpace);
#undef BSTONE_APPEND_FMT
#undef BSTONE_APPEND_CSPC
}

void VkR3rInfo::Impl::append_physical_device_surface_formats(VkPhysicalDevice physical_device)
{
	append_indentation();
	append_line("Surface formats:");
	increase_indentation();
	const auto surface_formats = vk_r3r_extract_array(
		context_->vkGetPhysicalDeviceSurfaceFormatsKHR, physical_device, context_->surface.get());
	std::size_t surface_format_number = 1;
	for (const auto& surface_format : surface_formats)
	{
		increase_indentation();
		append_indentation();
		append_number(surface_format_number);
		append_line(')');
		increase_indentation();
		append_physical_device_surface_format(surface_format);
		decrease_indentation();
		decrease_indentation();
		++surface_format_number;
	}
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device_surface_present_modes(VkPhysicalDevice physical_device)
{
	append_indentation();
	append_line("Surface present modes:");
	increase_indentation();
	const auto present_modes = vk_r3r_extract_array(
		context_->vkGetPhysicalDeviceSurfacePresentModesKHR, physical_device, context_->surface.get());
	for (const auto& present_mode : present_modes)
	{
		append_indentation();
		append_vk_present_mode_khr(present_mode);
		append_newline();
	}
	decrease_indentation();
}

void VkR3rInfo::Impl::append_physical_device(VkPhysicalDevice physical_device)
{
	increase_indentation();
	append_indentation();
	append_line("Properties:");
	increase_indentation();
	VkPhysicalDeviceProperties vk_physical_device_properties{};
	context_->vkGetPhysicalDeviceProperties(physical_device, &vk_physical_device_properties);
	append_physical_device_properties(vk_physical_device_properties);
	decrease_indentation();
	append_physical_device_features(physical_device);
	append_physical_device_memory_properties(physical_device);
	append_physical_device_surface_formats(physical_device);
	append_physical_device_surface_present_modes(physical_device);
	append_physical_device_formats_properties(physical_device);
	decrease_indentation();
}

// ==========================================================================

void VkR3rInfo::ImplDeleter::operator()(Impl* impl) const
{
	delete impl;
}

// ==========================================================================

VkR3rInfo::VkR3rInfo() = default;

VkR3rInfo::VkR3rInfo(sys::Logger& logger, const VkR3rContext& context)
	:
	impl_{new Impl(logger, context)}
{}

VkR3rInfo::~VkR3rInfo() = default;

void VkR3rInfo::log_validation_layers()
{
	impl_->log_validation_layers();
}

void VkR3rInfo::log_enabled_validation_layers()
{
	impl_->log_enabled_validation_layers();
}

void VkR3rInfo::log_enabled_extensions()
{
	impl_->log_enabled_extensions();
}

void VkR3rInfo::log_extensions()
{
	impl_->log_extensions();
}

void VkR3rInfo::log_surface_capabilities()
{
	impl_->log_surface_capabilities();
}

void VkR3rInfo::log_physical_devices()
{
	impl_->log_physical_devices();
}

} // namespace bstone
