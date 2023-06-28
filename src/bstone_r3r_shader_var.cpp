/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Variable

#include "bstone_exception.h"
#include "bstone_r3r_shader_var.h"

namespace bstone {

R3rShaderVarType R3rShaderVar::get_type() const noexcept
{
	return do_get_type();
}

R3rShaderVarTypeId R3rShaderVar::get_type_id() const noexcept
{
	return do_get_type_id();
}

int R3rShaderVar::get_index() const noexcept
{
	return do_get_index();
}

const std::string& R3rShaderVar::get_name() const noexcept
{
	return do_get_name();
}

int R3rShaderVar::get_input_index() const noexcept
{
	return do_get_input_index();
}

// ==========================================================================

void R3rShaderInt32Var::set_int32(std::int32_t value)
BSTONE_BEGIN_FUNC_TRY
	do_set_int32(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void R3rShaderFloat32Var::set_float32(float value)
BSTONE_BEGIN_FUNC_TRY
	do_set_float32(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void R3rShaderVec2Var::set_vec2(const float* value)
BSTONE_BEGIN_FUNC_TRY
	do_set_vec2(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void R3rShaderVec4Var::set_vec4(const float* value)
BSTONE_BEGIN_FUNC_TRY
	do_set_vec4(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void R3rShaderMat4Var::set_mat4(const float* value)
BSTONE_BEGIN_FUNC_TRY
	do_set_mat4(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

void R3rShaderR2SamplerVar::set_r2_sampler(std::int32_t value)
BSTONE_BEGIN_FUNC_TRY
	do_set_r2_sampler(value);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
