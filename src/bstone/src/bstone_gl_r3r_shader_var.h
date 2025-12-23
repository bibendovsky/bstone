/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader Variable

#ifndef BSTONE_GL_R3R_SHADER_VAR_INCLUDED
#define BSTONE_GL_R3R_SHADER_VAR_INCLUDED

#include "bstone_r3r_shader_var.h"
#include <string_view>

namespace bstone {

class GlR3rShaderStage;

// ==========================================================================

struct GlR3rShaderVarInitParam
{
	R3rShaderVarType type;
	R3rShaderVarTypeId type_id;
	int index;
	std::string_view name;
	int input_index;
	int gl_location;
};

class GlR3rShaderVar
{
public:
	static int get_unit_size(R3rShaderVarTypeId type_id);
};

using GlR3rShaderVarUPtr = std::unique_ptr<R3rShaderVar>;

// ==========================================================================

GlR3rShaderVarUPtr make_gl_r3r_shader_var(
	GlR3rShaderStage& shader_stage,
	const GlR3rShaderVarInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_SHADER_VAR_INCLUDED
