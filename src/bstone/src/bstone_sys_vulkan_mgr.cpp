/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager.

#include "bstone_sys_vulkan_mgr.h"

namespace bstone {
namespace sys {

bool VulkanMgr::is_vulkan_available() const
{
	return do_is_vulkan_available();
}

void* VulkanMgr::get_instance_proc_addr()
{
	return do_get_instance_proc_addr();
}

Span<const char*> VulkanMgr::get_required_extensions(Window& window)
{
	return do_get_required_extensions(window);
}

VkSurfaceKHR VulkanMgr::create_surface(Window& window, VkInstance vk_instance)
{
	return do_create_surface(window, vk_instance);
}

} // namespace sys
} // namespace bstone
