/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager (SDL2).

#include "bstone_sys_vulkan_mgr_sdl2.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>
#include "vulkan/vulkan.h"
#include "SDL_version.h"
#if SDL_VERSION_ATLEAST(2, 0, 6)
#define BSTONE_SDL2_SUPPORTS_VULKAN
#include "SDL_vulkan.h"
#endif

#include "bstone_single_pool_resource.h"
#include "bstone_sys_sdl2_version.h"
#include "bstone_sys_window_sdl2.h"

// ==========================================================================

namespace bstone {
namespace sys {

namespace {

class VulkanMgrSdl2 final : public VulkanMgr
{
public:
	explicit VulkanMgrSdl2(Logger& logger);
	~VulkanMgrSdl2() override;

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	using MemoryPool = SinglePoolResource<VulkanMgrSdl2>;
	using ExtensionStorage = std::vector<unsigned char>;

private:
	static MemoryPool memory_pool_;

private:
	Logger& logger_;
	bool is_vulkan_available_{};
	ExtensionStorage extension_storage_{};

private:
	bool do_is_vulkan_available() const override;
	void* do_get_instance_proc_addr() const override;
	std::span<const char*> do_get_required_extensions(Window& window) override;
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

VulkanMgrSdl2::MemoryPool VulkanMgrSdl2::memory_pool_{};

// --------------------------------------------------------------------------

VulkanMgrSdl2::VulkanMgrSdl2(Logger& logger)
	:
	logger_{logger}
{
	constexpr const char* const sdl2_version_message =
		"SDL v2.0.6 or higher required for Vulkan support.";

	logger_.log_information("Start up SDL2 Vulkan manager.");

#ifdef BSTONE_SDL2_SUPPORTS_VULKAN
	constexpr SDL_version sdl_v2_0_6{2, 0, 6};
	SDL_version sdl_rt_version;
	SDL_GetVersion(&sdl_rt_version);

	if (sdl_rt_version < sdl_v2_0_6)
	{
		logger_.log_information(sdl2_version_message);
		return;
	}

	if (SDL_Vulkan_LoadLibrary(nullptr) != 0)
	{
		std::string message{};
		message.reserve(512);
		message += "Failed to load the Vulkan loader library. ";
		message += SDL_GetError();
		logger_.log_information(message.c_str());
		return;
	}

	is_vulkan_available_ = true;
#else // BSTONE_SDL2_SUPPORTS_VULKAN
	logger_.log_information(sdl2_version_message);
#endif // BSTONE_SDL2_SUPPORTS_VULKAN
}

VulkanMgrSdl2::~VulkanMgrSdl2()
{
	logger_.log_information("Shut down SDL2 Vulkan manager.");

#ifdef BSTONE_SDL2_SUPPORTS_VULKAN
	if (impl_is_vulkan_available())
	{
		SDL_Vulkan_UnloadLibrary();
	}
#endif // BSTONE_SDL2_SUPPORTS_VULKAN
}

void* VulkanMgrSdl2::operator new(size_t size)
{
	return memory_pool_.allocate(static_cast<intptr_t>(size));
}

void VulkanMgrSdl2::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

bool VulkanMgrSdl2::do_is_vulkan_available() const
{
	return impl_is_vulkan_available();
}

void* VulkanMgrSdl2::do_get_instance_proc_addr() const
{
#ifdef BSTONE_SDL2_SUPPORTS_VULKAN
	ensure_is_vulkan_available();
	return SDL_Vulkan_GetVkGetInstanceProcAddr();
#else // BSTONE_SDL2_SUPPORTS_VULKAN
	vulkan_not_available();
#endif // BSTONE_SDL2_SUPPORTS_VULKAN
}

std::span<const char*> VulkanMgrSdl2::do_get_required_extensions(Window& window)
{
#ifdef BSTONE_SDL2_SUPPORTS_VULKAN
	ensure_is_vulkan_available();
	SDL_Window* const sdl2_window = get_sdl_window(window);
	unsigned int sdl_count;

	if (!SDL_Vulkan_GetInstanceExtensions(sdl2_window, &sdl_count, nullptr))
	{
		return std::span<const char*>{};
	}

	const int total_pointers_size = align16(static_cast<int>(sdl_count * sizeof(const char*)));

	using Pointers = std::vector<const char*>;
	Pointers sdl_pointers{};
	sdl_pointers.resize(sdl_count);

	if (!SDL_Vulkan_GetInstanceExtensions(sdl2_window, &sdl_count, sdl_pointers.data()))
	{
		return std::span<const char*>{};
	}

	sdl_pointers.resize(sdl_count);
	int total_strings_size = 0;

	for (unsigned int i = 0; i < sdl_count; ++i)
	{
		total_strings_size += align16(static_cast<int>(strlen(sdl_pointers[i]) + 1));
	}

	const int total_storage_size = total_pointers_size + total_strings_size;
	extension_storage_.clear();
	extension_storage_.resize(static_cast<size_t>(total_storage_size));
	const char** const pointers = reinterpret_cast<const char**>(extension_storage_.data());
	char* strings = reinterpret_cast<char*>(&extension_storage_[total_pointers_size]);

	for (unsigned int i = 0; i < sdl_count; ++i)
	{
		pointers[i] = strings;
		const char* const string = sdl_pointers[i];
		const int string_length = static_cast<int>(strlen(string));
		std::copy_n(string, string_length, strings);
		const int string_size = align16(string_length + 1);
		strings += string_size;
	}

	return std::span<const char*>{
		reinterpret_cast<const char**>(extension_storage_.data()),
		sdl_count
	};
#else // BSTONE_SDL2_SUPPORTS_VULKAN
	vulkan_not_available();
#endif // BSTONE_SDL2_SUPPORTS_VULKAN
}

VkSurfaceKHR VulkanMgrSdl2::do_create_surface(Window& window, VkInstance vk_instance)
{
#ifdef BSTONE_SDL2_SUPPORTS_VULKAN
	ensure_is_vulkan_available();
	SDL_Window* const sdl2_window = get_sdl_window(window);
	VkSurfaceKHR vk_surface_khr;

	if (!SDL_Vulkan_CreateSurface(sdl2_window, vk_instance, &vk_surface_khr))
	{
		std::string message{};
		message.reserve(512);
		message += "Failed to create Vulkan window surface. ";
		message += SDL_GetError();
		BSTONE_THROW_DYNAMIC_SOURCE(message.c_str());
	}

	return vk_surface_khr;
#else // BSTONE_SDL2_SUPPORTS_VULKAN
	vulkan_not_available();
#endif // BSTONE_SDL2_SUPPORTS_VULKAN
}

int VulkanMgrSdl2::align16(int value)
{
	return (value + 15) & (~15);
}

SDL_Window* VulkanMgrSdl2::get_sdl_window(Window& window)
{
	return static_cast<SDL_Window*>(static_cast<Sdl2WindowInternal&>(window).get_sdl_window());
}

[[noreturn]] void VulkanMgrSdl2::vulkan_not_available()
{
	BSTONE_THROW_STATIC_SOURCE("Vulkan not available.");
}

bool VulkanMgrSdl2::impl_is_vulkan_available() const
{
	return is_vulkan_available_;
}

void VulkanMgrSdl2::ensure_is_vulkan_available() const
{
	if (!impl_is_vulkan_available())
	{
		vulkan_not_available();
	}
}

} // namespace

// ==========================================================================

VulkanMgrUPtr make_vulkan_mgr(Logger& logger)
{
	return std::make_unique<VulkanMgrSdl2>(logger);
}

} // namespace sys
} // namespace bstone
