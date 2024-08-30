/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Shader Stage

#ifndef BSTONE_GL_R3R_SHADER_STAGE_INCLUDED
#define BSTONE_GL_R3R_SHADER_STAGE_INCLUDED

#include "bstone_gl_r3r_api.h"
#include "bstone_r3r_shader_stage.h"

namespace bstone {

class GlR3rContext;

// ==========================================================================

class GlR3rShaderStage : public R3rShaderStage
{
protected:
	GlR3rShaderStage();

public:
	~GlR3rShaderStage() override;
	virtual GlR3rContext& get_context() const noexcept = 0;

	virtual void set() = 0;
	virtual void detach_fragment_shader() = 0;
	virtual void detach_vertex_shader() = 0;
	virtual GLuint get_gl_name() const noexcept = 0;
};

// ==========================================================================

using GlR3rShaderStageUPtr = std::unique_ptr<GlR3rShaderStage>;

GlR3rShaderStageUPtr make_gl_r3r_shader_stage(GlR3rContext& context, const R3rShaderStageInitParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_SHADER_STAGE_INCLUDED
