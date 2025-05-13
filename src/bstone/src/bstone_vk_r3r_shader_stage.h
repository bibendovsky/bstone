/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader stage

#ifndef BSTONE_VK_R3R_SHADER_STAGE_INCLUDED
#define BSTONE_VK_R3R_SHADER_STAGE_INCLUDED

#include "bstone_r3r_shader_stage.h"
#include "vulkan/vulkan.h"

namespace bstone {

class VkR3rContext;
class VkR3rShader;

// ======================================

class VkR3rShaderStage : public R3rShaderStage
{
public:
	~VkR3rShaderStage() override {}

	std::uint32_t get_vk_image_binding() const;
	VkR3rShader* get_vertex_shader() const;
	VkR3rShader* get_fragment_shader() const;
	VkDescriptorSetLayout get_vk_descriptor_set_layout() const;
	VkDescriptorSet acquire_vk_descriptor_set();

protected:
	VkR3rShaderStage() {}

private:
	virtual std::uint32_t do_get_vk_image_binding() const = 0;
	virtual VkR3rShader* do_get_vertex_shader() const = 0;
	virtual VkR3rShader* do_get_fragment_shader() const = 0;
	virtual VkDescriptorSetLayout do_get_vk_descriptor_set_layout() const = 0;
	virtual VkDescriptorSet do_acquire_vk_descriptor_set() = 0;
};

// ======================================

using VkR3rShaderStageUPtr = std::unique_ptr<VkR3rShaderStage>;

VkR3rShaderStageUPtr make_vk_r3r_shader_stage(VkR3rContext& context, const R3rShaderStageInitParam& param);

} // namespace bstone

#endif // BSTONE_VK_R3R_SHADER_STAGE_INCLUDED
