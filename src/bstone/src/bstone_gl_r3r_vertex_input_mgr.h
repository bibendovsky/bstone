/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// OpenGL 3D Renderer: Vertex Input Manager

#ifndef BSTONE_GL_R3R_VERTEX_INPUT_MGR_INCLUDED
#define BSTONE_GL_R3R_VERTEX_INPUT_MGR_INCLUDED

#include <memory>
#include "bstone_r3r_vertex_input.h"

namespace bstone {

class GlR3rContext;

// ==========================================================================

class GlR3rVertexInputMgr
{
public:
	GlR3rVertexInputMgr() noexcept;
	virtual ~GlR3rVertexInputMgr();

	virtual GlR3rContext& get_context() const noexcept = 0;
	virtual R3rVertexInputUPtr create(const R3rCreateVertexInputParam& param) = 0;
	virtual void set(R3rVertexInput& vertex_input) = 0;
	virtual void bind_default_vao() = 0;
};

// ==========================================================================

using GlR3rVertexInputMgrUPtr = std::unique_ptr<GlR3rVertexInputMgr>;

GlR3rVertexInputMgrUPtr make_gl_r3r_vertex_input_mgr(GlR3rContext& context);

} // namespace bstone

#endif // BSTONE_GL_R3R_VERTEX_INPUT_MGR_INCLUDED
