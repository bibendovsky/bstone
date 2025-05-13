/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Vertex input

#ifndef BSTONE_VK_R3R_VERTEX_INPUT_INCLUDED
#define BSTONE_VK_R3R_VERTEX_INPUT_INCLUDED

#include "bstone_r3r_vertex_input.h"
#include "vulkan/vulkan.h"
#include <memory>

namespace bstone {

class VkR3rBuffer;
class VkR3rContext;

// ======================================

class VkR3rVertexInput : public R3rVertexInput
{
public:
	~VkR3rVertexInput() override {}

	VkR3rBuffer* get_index_buffer() const;
	VkR3rBuffer* get_vertex_buffer() const;
	VkBuffer get_vk_generic_buffer() const;
	const VkPipelineVertexInputStateCreateInfo& get_vk_create_info() const;

protected:
	VkR3rVertexInput() {}

private:
	virtual VkR3rBuffer* do_get_index_buffer() const = 0;
	virtual VkR3rBuffer* do_get_vertex_buffer() const = 0;
	virtual VkBuffer do_get_vk_generic_buffer() const = 0;
	virtual const VkPipelineVertexInputStateCreateInfo& do_get_vk_create_info() const = 0;
};

// ======================================

using VkR3rVertexInputUPtr = std::unique_ptr<VkR3rVertexInput>;

VkR3rVertexInputUPtr make_vk_r3r_vertex_input(VkR3rContext& context, const R3rCreateVertexInputParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_VERTEX_INPUT_INCLUDED
