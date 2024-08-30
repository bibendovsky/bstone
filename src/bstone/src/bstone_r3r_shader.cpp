/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Shader

#include "bstone_r3r_shader.h"

namespace bstone {

R3rShader::R3rShader() noexcept = default;

R3rShader::~R3rShader() = default;

R3rShaderType R3rShader::get_type() const noexcept
{
	return do_get_type();
}

} // namespace bstone
