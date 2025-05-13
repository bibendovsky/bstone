/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Sampler

#ifndef BSTONE_VK_R3R_SAMPLER_INCLUDED
#define BSTONE_VK_R3R_SAMPLER_INCLUDED

#include "vulkan/vulkan.h"
#include "bstone_r3r_sampler.h"

namespace bstone {

class VkR3rContext;

// ======================================

class VkR3rSampler : public R3rSampler
{
public:
	~VkR3rSampler() override {}

	VkSampler get_vk_sampler() const;

protected:
	VkR3rSampler() {}

private:
	virtual VkSampler do_get_vk_sampler() const = 0;
};

// ======================================

using VkR3rSamplerUPtr = std::unique_ptr<VkR3rSampler>;

VkR3rSamplerUPtr make_vk_r3r_sampler(VkR3rContext& context, const R3rSamplerInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_SAMPLER_INCLUDED
