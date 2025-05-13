/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: 2D texture

#ifndef BSTONE_VK_R3R_R2_TEXTURE_INCLUDED
#define BSTONE_VK_R3R_R2_TEXTURE_INCLUDED

#include "bstone_r3r_r2_texture.h"
#include "bstone_r3r_sampler.h"
#include "vulkan/vulkan.h"
#include <memory>

namespace bstone {

class VkR3rContext;

// ======================================

class VkR3rR2Texture : public R3rR2Texture
{
public:
	~VkR3rR2Texture() override {}

	VkImageView get_vk_image_view() const;

protected:
	VkR3rR2Texture() {}

private:
	virtual VkImageView do_get_vk_image_view() const = 0;
};

// ======================================

using VkR3rR2TextureUPtr = std::unique_ptr<VkR3rR2Texture>;

VkR3rR2TextureUPtr make_vk_r3r_r2_texture(VkR3rContext& context, const R3rR2TextureInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_R2_TEXTURE_INCLUDED
