/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader

#ifndef BSTONE_GL_R3R_SHADER_INCLUDED
#define BSTONE_GL_R3R_SHADER_INCLUDED

#include "bstone_gl_r3r_api.h"
#include "bstone_r3r_shader.h"

namespace bstone {

class GlR3rShaderMgr;
class GlR3rShaderStage;

// ==========================================================================

class GlR3rShader : public R3rShader
{
protected:
	GlR3rShader();

public:
	~GlR3rShader() override;


	virtual GLuint get_gl_name() const noexcept = 0;

	virtual void attach_to_shader_stage(GlR3rShaderStage* shader_stage) = 0;
};

using GlR3rShaderUPtr = std::unique_ptr<GlR3rShader>;

// ==========================================================================

GlR3rShaderUPtr make_gl_r3r_shader(const R3rShaderInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_SHADER_INCLUDED
