/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Vulkan 3D renderer: Shader variable

#ifndef BSTONE_VK_R3R_SHADER_VAR_INCLUDED
#define BSTONE_VK_R3R_SHADER_VAR_INCLUDED

#include "bstone_r3r_shader_var.h"
#include <memory>

namespace bstone {

using R3rShaderVarUPtr = std::unique_ptr<R3rShaderVar>;

R3rShaderVarUPtr make_vk_r3r_shader_var(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name,
	void* mapped_memory);

} // namespace bstone

#endif // BSTONE_VK_R3R_SHADER_VAR_INCLUDED
