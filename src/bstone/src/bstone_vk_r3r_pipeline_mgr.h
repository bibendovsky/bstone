/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Pipeline manager

#ifndef BSTONE_VK_R3R_PIPELINE_MGR_INCLUDED
#define BSTONE_VK_R3R_PIPELINE_MGR_INCLUDED

#include <memory>

namespace bstone {

class VkR3rContext;
class VkR3rPipeline;

// ======================================

class VkR3rPipelineMgr
{
public:
	VkR3rPipelineMgr() = default;
	virtual ~VkR3rPipelineMgr() = default;

	virtual void clear() = 0;
	virtual VkR3rPipeline* acquire_pipeline() = 0;
};

// ======================================

using VkR3rPipelineMgrUPtr = std::unique_ptr<VkR3rPipelineMgr>;

VkR3rPipelineMgrUPtr make_vk_r3r_pipeline_mgr(VkR3rContext& context);

} // namespace bstone

#endif // BSTONE_VK_R3R_PIPELINE_MGR_INCLUDED
