/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Enum string constants

#ifndef BSTONE_VK_R3R_ENUM_STRINGS_INCLUDED
#define BSTONE_VK_R3R_ENUM_STRINGS_INCLUDED

#include "vulkan/vulkan.h"

namespace bstone {

class VkR3rEnumStrings
{
public:
	static const char* get_VkResult(VkResult value);
	static const char* get_VkVendorId(VkVendorId value);
	static const char* get_VkPhysicalDeviceType(VkPhysicalDeviceType value);
	static const char* get_VkFormat(VkFormat value);
	static const char* get_VkPresentModeKHR(VkPresentModeKHR value);
	static const char* get_VkColorSpaceKHR(VkColorSpaceKHR value);
	static const char* get_VkFormatFeatureFlagBits(VkFormatFeatureFlagBits flag);
	static const char* get_VkSampleCountFlagBits(VkSampleCountFlagBits flag);
	static const char* get_VkImageUsageFlagBits(VkImageUsageFlagBits flag);
	static const char* get_VkMemoryHeapFlagBits(VkMemoryHeapFlagBits flag);
	static const char* get_VkMemoryPropertyFlagBits(VkMemoryPropertyFlagBits flag);
	static const char* get_VkSurfaceTransformFlagBitsKHR(VkSurfaceTransformFlagBitsKHR flag);
	static const char* get_VkCompositeAlphaFlagBitsKHR(VkCompositeAlphaFlagBitsKHR flag);
};

} // namespace bstone

#endif // BSTONE_VK_R3R_ENUM_STRINGS_INCLUDED
