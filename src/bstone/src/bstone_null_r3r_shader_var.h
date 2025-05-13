/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader variable

#ifndef BSTONE_NULL_R3R_SHADER_VAR_INCLUDED
#define BSTONE_NULL_R3R_SHADER_VAR_INCLUDED

#include <memory>
#include "bstone_r3r_shader_var.h"

// ==========================================================================

namespace bstone {

using NullR3rShaderVarUPtr = std::unique_ptr<R3rShaderVar>;

NullR3rShaderVarUPtr make_null_r3r_shader_var(
	R3rShaderVarType type,
	R3rShaderVarTypeId type_id,
	int index,
	const char* name);

} // namespace bstone

#endif // BSTONE_NULL_R3R_SHADER_VAR_INCLUDED
