/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Stage

#include "bstone_r3r_shader_stage.h"

namespace bstone {

R3rShaderStage::R3rShaderStage() noexcept = default;

R3rShaderStage::~R3rShaderStage() = default;

R3rShaderVar* R3rShaderStage::find_var(const char* name) noexcept
{
	return do_find_var(name);
}

R3rShaderInt32Var* R3rShaderStage::find_int32_var(const char* name) noexcept
{
	return do_find_int32_var(name);
}

R3rShaderFloat32Var* R3rShaderStage::find_float32_var(const char* name) noexcept
{
	return do_find_float32_var(name);
}

R3rShaderVec2Var* R3rShaderStage::find_vec2_var(const char* name) noexcept
{
	return do_find_vec2_var(name);
}

R3rShaderVec4Var* R3rShaderStage::find_vec4_var(const char* name) noexcept
{
	return do_find_vec4_var(name);
}

R3rShaderMat4Var* R3rShaderStage::find_mat4_var(const char* name) noexcept
{
	return do_find_mat4_var(name);
}

R3rShaderR2SamplerVar* R3rShaderStage::find_r2_sampler_var(const char* name) noexcept
{
	return do_find_r2_sampler_var(name);
}

} // namespace bstone
