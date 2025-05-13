/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader

#ifndef BSTONE_VK_R3R_SHADER_INCLUDED
#define BSTONE_VK_R3R_SHADER_INCLUDED

#include "vulkan/vulkan.h"
#include "bstone_r3r_shader.h"

namespace bstone {

class VkR3rContext;

// ======================================

class VkR3rShader : public R3rShader
{
public:
	~VkR3rShader() override {}

	VkShaderModule get_vk_shader_module() const;

protected:
	VkR3rShader() {}

private:
	virtual VkShaderModule do_get_vk_shader_module() const = 0;
};

using VkR3rShaderUPtr = std::unique_ptr<VkR3rShader>;

// ======================================

VkR3rShaderUPtr make_vk_r3r_shader(VkR3rContext& context, const R3rShaderInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_SHADER_INCLUDED
