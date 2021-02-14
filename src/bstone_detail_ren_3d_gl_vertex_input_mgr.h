/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
protected:
	Ren3dGlVertexInputMgr() = default;


public:
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
