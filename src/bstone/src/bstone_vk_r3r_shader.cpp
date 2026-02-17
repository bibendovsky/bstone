/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader

#include "bstone_vk_r3r_shader.h"
#include "bstone_exception.h"
#include "bstone_r3r_limits.h"
#include "bstone_vk_r3r_context.h"
#include "bstone_vk_r3r_raii.h"
#include <cstddef>
#include <cstdint>

namespace bstone {

namespace {

class VkR3rShaderImpl final : public VkR3rShader
{
public:
	VkR3rShaderImpl(VkR3rContext& context, const R3rShaderInitParam& param);
	~VkR3rShaderImpl() override {}

	R3rShaderType get_type() const override;
	VkShaderModule get_vk_shader_module() const override;

private:
	VkR3rContext& context_;
	R3rShaderType type_{};
	VkR3rShaderModuleResource shader_{};
};

// --------------------------------------

VkR3rShaderImpl::VkR3rShaderImpl(VkR3rContext& context, const R3rShaderInitParam& param)
	:
	context_{context},
	type_{param.type}
{
	const VkShaderModuleCreateInfo vk_shader_module_create_info
	{
		/* sType */    VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		/* pNext */    nullptr,
		/* flags */    VkShaderModuleCreateFlags{},
		/* codeSize */ static_cast<std::size_t>(param.source.size),
		/* pCode */    static_cast<const std::uint32_t*>(param.source.data),
	};
	VkShaderModule vk_shader_module{};
	const VkResult vk_result = context_.vkCreateShaderModule(
		context_.device.get(), &vk_shader_module_create_info, nullptr, &vk_shader_module);
	VkR3rContext::ensure_success_vk_result(vk_result, "vkCreateShaderModule");
	shader_.reset(vk_shader_module, VkR3rShaderModuleDeleter{context_});
}

R3rShaderType VkR3rShaderImpl::get_type() const
{
	return type_;
}

VkShaderModule VkR3rShaderImpl::get_vk_shader_module() const
{
	return shader_.get();
}

} // namespace

// ======================================

VkR3rShaderUPtr make_vk_r3r_shader(VkR3rContext& context, const R3rShaderInitParam& param)
{
	return std::make_unique<VkR3rShaderImpl>(context, param);
}

} // namespace bstone
