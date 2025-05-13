/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Pipeline

#ifndef BSTONE_VK_R3R_PIPELINE_INCLUDED
#define BSTONE_VK_R3R_PIPELINE_INCLUDED

#include <memory>

namespace bstone {

class VkR3rContext;

// ======================================

struct VkR3rPipelineDrawIndexedParam
{
	int vertex_count;
	int index_byte_depth;
	int index_buffer_offset;
	int index_offset;
};

// ======================================

class VkR3rPipeline
{
public:
	VkR3rPipeline() {}
	virtual ~VkR3rPipeline() {}

	void draw_indexed(const VkR3rPipelineDrawIndexedParam& param);

	virtual void do_draw_indexed(const VkR3rPipelineDrawIndexedParam& param) = 0;
};

// ======================================

using VkR3rPipelineUPtr = std::unique_ptr<VkR3rPipeline>;

VkR3rPipelineUPtr make_vk_r3r_pipeline(VkR3rContext& context);

} // namespace bstone

#endif // BSTONE_VK_R3R_PIPELINE_INCLUDED
