/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Buffer

#ifndef BSTONE_VK_R3R_BUFFER_INCLUDED
#define BSTONE_VK_R3R_BUFFER_INCLUDED

#include "vulkan/vulkan.h"
#include "bstone_r3r_buffer.h"
#include "bstone_vk_r3r_raii.h"
#include <memory>

namespace bstone {

class VkR3rContext;

// ======================================

class VkR3rBuffer : public R3rBuffer
{
public:
	~VkR3rBuffer() override {}
	VkBuffer get_vk_buffer() const;

protected:
	VkR3rBuffer() {}

private:
	virtual VkBuffer do_get_vk_buffer() const = 0;
};

// ======================================

using VkR3rBufferUPtr = std::unique_ptr<VkR3rBuffer>;

VkR3rBufferUPtr make_vk_r3r_buffer(VkR3rContext& context, const R3rBufferInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_BUFFER_INCLUDED
