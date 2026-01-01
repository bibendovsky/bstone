/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager (SDL)

#include "bstone_sys_vulkan_mgr_sdl.h"
#include "bstone_exception.h"
#include "bstone_sys_window_sdl.h"
#include <format>
#include <string>
#include "vulkan/vulkan.h"
#include "SDL3/SDL_vulkan.h"

namespace bstone::sys {

namespace {

class VulkanMgrSdl final : public VulkanMgr
{
public:
	explicit VulkanMgrSdl(Logger& logger);
	~VulkanMgrSdl() override;

private:
	Logger& logger_;
	bool is_vulkan_available_{};

	bool do_is_vulkan_available() const override;
	VulkanMgrSymbolFunc do_get_instance_proc_addr() const override;
	std::span<const char* const> do_get_required_extensions(Window& window) override;
	VkSurfaceKHR do_create_surface(Window& window, VkInstance vk_instance) override;

	[[noreturn]] static void fail_sdl_func(const char* sdl_func_name);
	static SDL_Window* get_sdl_window(Window& window);
	[[noreturn]] static void vulkan_not_available();
	bool impl_is_vulkan_available() const;
	void ensure_is_vulkan_available() const;
};

// --------------------------------------

VulkanMgrSdl::VulkanMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Starting SDL Vulkan manager.");
	if (!SDL_Vulkan_LoadLibrary(nullptr))
	{
		const std::string message = std::format("[{}] {}", "SDL_Vulkan_LoadLibrary", SDL_GetError());
		logger_.log_information(message.c_str());
		return;
	}
	is_vulkan_available_ = true;
	logger_.log_information("SDL Vulkan manager has started.");
}

VulkanMgrSdl::~VulkanMgrSdl()
{
	logger_.log_information("Shut down SDL Vulkan manager.");
	if (impl_is_vulkan_available())
	{
		SDL_Vulkan_UnloadLibrary();
	}
}

bool VulkanMgrSdl::do_is_vulkan_available() const
{
	return impl_is_vulkan_available();
}

VulkanMgrSymbolFunc VulkanMgrSdl::do_get_instance_proc_addr() const
{
	ensure_is_vulkan_available();
	return reinterpret_cast<VulkanMgrSymbolFunc>(SDL_Vulkan_GetVkGetInstanceProcAddr());
}

std::span<const char* const> VulkanMgrSdl::do_get_required_extensions(Window& window)
{
	ensure_is_vulkan_available();
	Uint32 sdl_count;
	if (const char* const* const sdl_extensions_strings = SDL_Vulkan_GetInstanceExtensions(&sdl_count);
		sdl_extensions_strings != nullptr)
	{
		return std::span<const char* const>{sdl_extensions_strings, sdl_count};
	}
	return std::span<const char* const>{};
}

VkSurfaceKHR VulkanMgrSdl::do_create_surface(Window& window, VkInstance vk_instance)
{
	ensure_is_vulkan_available();
	SDL_Window* const sdl_window = get_sdl_window(window);
	VkSurfaceKHR vk_surface_khr;
	if (!SDL_Vulkan_CreateSurface(sdl_window, vk_instance, nullptr, &vk_surface_khr))
	{
		fail_sdl_func("SDL_Vulkan_CreateSurface");
	}
	return vk_surface_khr;
}

[[noreturn]] void VulkanMgrSdl::fail_sdl_func(const char* sdl_func_name)
{
	const std::string message = std::format("[{}] {}", sdl_func_name, SDL_GetError());
	BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
}

SDL_Window* VulkanMgrSdl::get_sdl_window(Window& window)
{
	return static_cast<SDL_Window*>(static_cast<SdlWindowInternal&>(window).get_sdl_window());
}

[[noreturn]] void VulkanMgrSdl::vulkan_not_available()
{
	BSTONE_THROW_STATIC_SOURCE("Vulkan not available.");
}

bool VulkanMgrSdl::impl_is_vulkan_available() const
{
	return is_vulkan_available_;
}

void VulkanMgrSdl::ensure_is_vulkan_available() const
{
	if (!impl_is_vulkan_available())
	{
		vulkan_not_available();
	}
}

} // namespace

// ======================================

VulkanMgrUPtr make_vulkan_mgr_sdl(Logger& logger)
{
	return std::make_unique<VulkanMgrSdl>(logger);
}

} // namespace bstone::sys
