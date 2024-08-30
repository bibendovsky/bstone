/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader Variable

#ifndef BSTONE_GL_R3R_SHADER_VAR_INCLUDED
#define BSTONE_GL_R3R_SHADER_VAR_INCLUDED

#include "bstone_r3r_shader_var.h"
#include "bstone_string_view.h"

namespace bstone {

class GlR3rShaderStage;

// ==========================================================================

struct GlR3rShaderVarInitParam
{
	R3rShaderVarType type;
	R3rShaderVarTypeId type_id;
	int value_size;
	int index;
	StringView name;
	int input_index;
	int gl_location;
};

class GlR3rShaderVar :
	public virtual R3rShaderInt32Var,
	public virtual R3rShaderFloat32Var,
	public virtual R3rShaderVec2Var,
	public virtual R3rShaderVec4Var,
	public virtual R3rShaderMat4Var,
	public virtual R3rShaderR2SamplerVar
{
protected:
	GlR3rShaderVar();

public:
	~GlR3rShaderVar() override;

	static int get_unit_size(R3rShaderVarTypeId type_id);
};

using GlR3rShaderVarUPtr = std::unique_ptr<GlR3rShaderVar>;

// ==========================================================================

GlR3rShaderVarUPtr make_gl_r3r_shader_var(
	GlR3rShaderStage& shader_stage,
	const GlR3rShaderVarInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_SHADER_VAR_INCLUDED
