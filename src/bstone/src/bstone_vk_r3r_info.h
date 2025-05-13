/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Outputs various information about Vulkan.

#ifndef BSTONE_VK_R3R_INFO_INCLUDED
#define BSTONE_VK_R3R_INFO_INCLUDED

#include <memory>

namespace bstone {

namespace sys {

class Logger;

} // namespace sys

class VkR3rContext;

// ======================================

class VkR3rInfo
{
public:
	VkR3rInfo(sys::Logger& logger, const VkR3rContext& context);
	VkR3rInfo(const VkR3rInfo&) = delete;
	VkR3rInfo& operator=(const VkR3rInfo&) = delete;
	~VkR3rInfo();

	void log_validation_layers();
	void log_enabled_validation_layers();
	void log_enabled_extensions();
	void log_extensions();
	void log_surface_capabilities();
	void log_physical_devices();

private:
	class Impl;
	struct ImplDeleter
	{
		void operator()(Impl* impl) const;
	};
	using ImplUPtr = std::unique_ptr<Impl, ImplDeleter>;

	ImplUPtr impl_{};

	VkR3rInfo();
};

} // namespace bstone

#endif // BSTONE_VK_R3R_INFO_INCLUDED
