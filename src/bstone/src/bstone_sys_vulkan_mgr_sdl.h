/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan manager (SDL)

#ifndef BSTONE_SYS_VULKAN_MGR_SDL_INCLUDED
#define BSTONE_SYS_VULKAN_MGR_SDL_INCLUDED

#include "bstone_sys_logger.h"
#include "bstone_sys_vulkan_mgr.h"

namespace bstone::sys {

VulkanMgrUPtr make_vulkan_mgr_sdl(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_VULKAN_MGR_SDL_INCLUDED
