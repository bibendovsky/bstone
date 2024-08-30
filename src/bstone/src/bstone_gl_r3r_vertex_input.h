/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Vertex Input

#ifndef BSTONE_GL_R3R_VERTEX_INPUT_INCLUDED
#define BSTONE_GL_R3R_VERTEX_INPUT_INCLUDED

#include "bstone_gl_r3r_device_features.h"
#include "bstone_r3r_vertex_input.h"

namespace bstone {

class GlR3rVertexInputMgr;

// =========================================================================

class GlR3rVertexInput : public R3rVertexInput
{
protected:
	GlR3rVertexInput();

public:
	~GlR3rVertexInput() override;

	virtual void bind() = 0;
	virtual void bind_vao() = 0;

	virtual R3rBuffer* get_index_buffer() const noexcept = 0;
};

// =========================================================================

using GlR3rVertexInputUPtr = std::unique_ptr<GlR3rVertexInput>;

GlR3rVertexInputUPtr make_gl_r3r_vertex_input(
	GlR3rVertexInputMgr& vertex_input_manager,
	const R3rCreateVertexInputParam& param);

} // namespace bstone

#endif // BSTONE_GL_R3R_VERTEX_INPUT_INCLUDED
