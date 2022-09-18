/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL vertex input manager (implementation interface).
//


#ifndef BSTONE_DETAIL_REN_3D_GL_VERTEX_INPUT_MGR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_VERTEX_INPUT_MGR_INCLUDED


#include <memory>

#include "bstone_ren_3d_vertex_input.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;


// ==========================================================================
// Ren3dGlVertexInputMgr
//

class Ren3dGlVertexInputMgr
{
public:
	Ren3dGlVertexInputMgr() noexcept = default;

	virtual ~Ren3dGlVertexInputMgr() = default;


	virtual Ren3dGlContextPtr get_context() const noexcept = 0;

	virtual Ren3dVertexInputUPtr create(
		const Ren3dCreateVertexInputParam& param) = 0;

	virtual void set(
		const Ren3dVertexInputPtr vertex_input) = 0;

	virtual void bind_default_vao() = 0;
}; // Ren3dGlVertexInputMgr

using Ren3dGlVertexInputMgrPtr = Ren3dGlVertexInputMgr*;
using Ren3dGlVertexInputMgrUPtr = std::unique_ptr<Ren3dGlVertexInputMgr>;

//
// Ren3dGlVertexInputMgr
// ==========================================================================


// ==========================================================================
// Ren3dGlVertexInputMgrFactory
//

struct Ren3dGlVertexInputMgrFactory
{
	static Ren3dGlVertexInputMgrUPtr create(
		const Ren3dGlContextPtr context);
}; // Ren3dGlVertexInputMgrFactory

//
// Ren3dGlVertexInputMgrFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_VERTEX_INPUT_MGR_INCLUDED
