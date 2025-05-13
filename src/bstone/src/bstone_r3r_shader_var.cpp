/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader Variable

#include "bstone_exception.h"
#include "bstone_r3r_shader_var.h"

namespace bstone {

R3rShaderVarType R3rShaderVar::get_type() const
{
	return do_get_type();
}

R3rShaderVarTypeId R3rShaderVar::get_type_id() const
{
	return do_get_type_id();
}

int R3rShaderVar::get_index() const
{
	return do_get_index();
}

const std::string& R3rShaderVar::get_name() const
{
	return do_get_name();
}

void R3rShaderVar::set_int32(int32_t value)
try {
	do_set_int32(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_float32(float value)
try {
	do_set_float32(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_vec2(const float* value)
try {
	do_set_vec2(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_vec3(const float* value)
try {
	do_set_vec3(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_vec4(const float* value)
try {
	do_set_vec4(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_mat4(const float* value)
try {
	do_set_mat4(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void R3rShaderVar::set_r2_sampler(int32_t value)
try {
	do_set_r2_sampler(value);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
