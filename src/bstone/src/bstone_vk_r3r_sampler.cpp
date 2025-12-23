/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Sampler

#include "bstone_vk_r3r_sampler.h"
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_raii.h"
#include <cstddef>
#include <cstdint>
#include <algorithm>

// ======================================

namespace bstone {

namespace {

class VkR3rSamplerImpl final : public VkR3rSampler
{
public:
	VkR3rSamplerImpl(VkR3rContext& context, const R3rSamplerInitParam& param);
	~VkR3rSamplerImpl() override {}

	void* operator new(std::size_t size);
	void operator delete(void* ptr);

private:
	static const int min_anisotropy;

	void do_update(const R3rSamplerUpdateParam& param) override;
	const R3rSamplerState& do_get_state() const noexcept override;
	VkSampler do_get_vk_sampler() const override;

	using MemoryPool = FixedPoolResource<VkR3rSamplerImpl, R3rLimits::max_samplers()>;

	static MemoryPool memory_pool_;

	VkR3rContext& context_;
	R3rSamplerState state_{};
	VkR3rSamplerResource sampler_resource_{};

	static VkFilter map_filter(R3rFilterType filter);
	static VkSamplerMipmapMode map_mipmap_mode(R3rMipmapMode mipmap_mode);
	static VkSamplerAddressMode map_address_mode(R3rAddressMode address_mode);
	float clamp_anisotropy(int anisotropy) const;
	void update_internal();
};

// --------------------------------------

const int VkR3rSamplerImpl::min_anisotropy = 1;
VkR3rSamplerImpl::MemoryPool VkR3rSamplerImpl::memory_pool_{};

// --------------------------------------

VkR3rSamplerImpl::VkR3rSamplerImpl(VkR3rContext& context, const R3rSamplerInitParam& param)
try
	:
	context_{context},
	state_{param.state}
{
	update_internal();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void* VkR3rSamplerImpl::operator new(std::size_t size)
try {
	return memory_pool_.allocate(static_cast<std::intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void VkR3rSamplerImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void VkR3rSamplerImpl::do_update(const R3rSamplerUpdateParam& param)
{
	if (param.state.mag_filter == state_.mag_filter &&
		param.state.min_filter == state_.min_filter &&
		param.state.mipmap_mode == state_.mipmap_mode &&
		param.state.address_mode_u == state_.address_mode_u &&
		param.state.address_mode_v == state_.address_mode_v &&
		param.state.anisotropy == state_.anisotropy)
	{
		return;
	}

	state_ = param.state;
	update_internal();
}

const R3rSamplerState& VkR3rSamplerImpl::do_get_state() const noexcept
{
	return state_;
}

VkSampler VkR3rSamplerImpl::do_get_vk_sampler() const
{
	return sampler_resource_.get();
}

VkFilter VkR3rSamplerImpl::map_filter(R3rFilterType filter)
{
	switch (filter)
	{
		case R3rFilterType::nearest: return VK_FILTER_NEAREST;
		case R3rFilterType::linear: return VK_FILTER_LINEAR;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown filter.");
	}
}

VkSamplerMipmapMode VkR3rSamplerImpl::map_mipmap_mode(R3rMipmapMode mipmap_mode)
{
	switch (mipmap_mode)
	{
		case R3rMipmapMode::nearest: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		case R3rMipmapMode::linear: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown mipmap mode.");
	}
}

VkSamplerAddressMode VkR3rSamplerImpl::map_address_mode(R3rAddressMode address_mode)
{
	switch (address_mode)
	{
		case R3rAddressMode::clamp: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case R3rAddressMode::repeat: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown address mode.");
	}
}

float VkR3rSamplerImpl::clamp_anisotropy(int anisotropy) const
{
	return std::min(
		static_cast<float>(std::max(anisotropy, min_anisotropy)),
		context_.physical_device_properties.limits.maxSamplerAnisotropy);
}

void VkR3rSamplerImpl::update_internal()
{
	sampler_resource_.reset();
	const bool is_mipmap = state_.mipmap_mode != R3rMipmapMode::none;
	VkSamplerMipmapMode vk_sampler_mipmap_mode;
	float vk_max_lod;
	VkBool32 vk_anisotropy_enable;
	float vk_max_anisotropy;
	if (is_mipmap)
	{
		vk_sampler_mipmap_mode = map_mipmap_mode(state_.mipmap_mode);
		vk_max_lod = VK_LOD_CLAMP_NONE;

		const float clamped_anisotropy = clamp_anisotropy(state_.anisotropy);

		vk_anisotropy_enable =
			context_.physical_device_features.samplerAnisotropy != VK_FALSE &&
			context_.physical_device_properties.limits.maxSamplerAnisotropy > 1.0F &&
			clamped_anisotropy > 1.0F;

		vk_max_anisotropy = clamped_anisotropy;
	}
	else
	{
		vk_sampler_mipmap_mode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		vk_max_lod = 0.25F;
		vk_anisotropy_enable = false;
		vk_max_anisotropy = 0.0F;
	}
	const VkSamplerCreateInfo vk_sampler_create_info
	{
		/* sType */                   VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		/* pNext */                   nullptr,
		/* flags */                   VkSamplerCreateFlags{},
		/* magFilter */               map_filter(state_.mag_filter),
		/* minFilter */               map_filter(state_.min_filter),
		/* mipmapMode */              vk_sampler_mipmap_mode,
		/* addressModeU */            map_address_mode(state_.address_mode_u),
		/* addressModeV */            map_address_mode(state_.address_mode_v),
		/* addressModeW */            VK_SAMPLER_ADDRESS_MODE_REPEAT,
		/* mipLodBias */              0.0F,
		/* anisotropyEnable */        vk_anisotropy_enable,
		/* maxAnisotropy */           vk_max_anisotropy,
		/* compareEnable */           false,
		/* compareOp */               VkCompareOp{},
		/* minLod */                  0.0F,
		/* maxLod */                  vk_max_lod,
		/* borderColor */             VK_BORDER_COLOR_INT_OPAQUE_BLACK,
		/* unnormalizedCoordinates */ false,
	};
	VkSampler vk_sampler{};
	const VkResult vk_result = context_.vkCreateSampler(
		context_.device.get(), &vk_sampler_create_info, nullptr, &vk_sampler);
	VkR3rContext::ensure_success_vk_result(vk_result, "vkCreateSampler");
	sampler_resource_.reset(vk_sampler, VkR3rSamplerDeleter{context_});
}

} // namespace

// ======================================

VkSampler VkR3rSampler::get_vk_sampler() const
{
	return do_get_vk_sampler();
}

// ======================================

VkR3rSamplerUPtr make_vk_r3r_sampler(VkR3rContext& context, const R3rSamplerInitParam& param)
{
	return std::make_unique<VkR3rSamplerImpl>(context, param);
}

} // namespace bstone
