/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager

#ifndef BSTONE_SYS_VULKAN_MGR_INCLUDED
#define BSTONE_SYS_VULKAN_MGR_INCLUDED

#include "bstone_sys_window.h"
#include <memory>
#include <span>
#include "vulkan/vulkan.h"

namespace bstone::sys {

using VulkanMgrSymbolFunc = void (*)();

class VulkanMgr
{
public:
	VulkanMgr() = default;
	virtual ~VulkanMgr() = default;

	bool is_vulkan_available() const;
	VulkanMgrSymbolFunc get_instance_proc_addr();
	std::span<const char* const> get_required_extensions(Window& window);
	VkSurfaceKHR create_surface(Window& window, VkInstance vk_instance);

private:
	virtual bool do_is_vulkan_available() const = 0;
	virtual VulkanMgrSymbolFunc do_get_instance_proc_addr() const = 0;
	virtual std::span<const char* const> do_get_required_extensions(Window& window) = 0;
	virtual VkSurfaceKHR do_create_surface(Window& window, VkInstance vk_instance) = 0;
};

// ======================================

using VulkanMgrUPtr = std::unique_ptr<VulkanMgr>;

} // namespace bstone::sys

#endif // BSTONE_SYS_VULKAN_MGR_INCLUDED
