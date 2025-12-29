/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager (SDL).

#include "bstone_sys_vulkan_mgr_sdl.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <string>
#include "vulkan/vulkan.h"
#include "SDL3/SDL_vulkan.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_window_sdl.h"

// ==========================================================================

namespace bstone {
namespace sys {

namespace {

class VulkanMgrSdl final : public VulkanMgr
{
public:
	explicit VulkanMgrSdl(Logger& logger);
	~VulkanMgrSdl() override;

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	using MemoryPool = SinglePoolResource<VulkanMgrSdl>;

private:
	static MemoryPool memory_pool_;

private:
	Logger& logger_;
	bool is_vulkan_available_{};

private:
	bool do_is_vulkan_available() const override;
	VulkanMgrSymbolFunc do_get_instance_proc_addr() const override;
	std::span<const char* const> do_get_required_extensions(Window& window) override;
	VkSurfaceKHR do_create_surface(Window& window, VkInstance vk_instance) override;

private:
	static int align16(int value);
	static SDL_Window* get_sdl_window(Window& window);

private:
	[[noreturn]] static void vulkan_not_available();

private:
	bool impl_is_vulkan_available() const;
	void ensure_is_vulkan_available() const;
};

// --------------------------------------------------------------------------

VulkanMgrSdl::MemoryPool VulkanMgrSdl::memory_pool_{};

// --------------------------------------------------------------------------

VulkanMgrSdl::VulkanMgrSdl(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up SDL Vulkan manager.");
	if (!SDL_Vulkan_LoadLibrary(nullptr))
	{
		std::string message{};
		message.reserve(512);
		message += "Failed to load the Vulkan loader library. ";
		message += SDL_GetError();
		logger_.log_information(message.c_str());
		return;
	}
	is_vulkan_available_ = true;
}

VulkanMgrSdl::~VulkanMgrSdl()
{
	logger_.log_information("Shut down SDL Vulkan manager.");
	if (impl_is_vulkan_available())
	{
		SDL_Vulkan_UnloadLibrary();
	}
}

void* VulkanMgrSdl::operator new(size_t size)
{
	return memory_pool_.allocate(static_cast<intptr_t>(size));
}

void VulkanMgrSdl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
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
		std::string message{};
		message.reserve(512);
		message += "Failed to create Vulkan window surface. ";
		message += SDL_GetError();
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}

	return vk_surface_khr;
}

int VulkanMgrSdl::align16(int value)
{
	return (value + 15) & (~15);
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

// ==========================================================================

VulkanMgrUPtr make_sdl_vulkan_mgr(Logger& logger)
{
	return std::make_unique<VulkanMgrSdl>(logger);
}

} // namespace sys
} // namespace bstone
