/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader

#ifndef BSTONE_NULL_R3R_SHADER_INCLUDED
#define BSTONE_NULL_R3R_SHADER_INCLUDED

#include "bstone_r3r_shader.h"

// ==========================================================================

namespace bstone {

using NullR3rShaderUPtr = std::unique_ptr<R3rShader>;

NullR3rShaderUPtr make_null_r3r_shader(const R3rShaderInitParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_SHADER_INCLUDED
