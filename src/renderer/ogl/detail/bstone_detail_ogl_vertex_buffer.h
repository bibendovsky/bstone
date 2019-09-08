/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// OpenGL vertex buffer object (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED
#define BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglBufferManager;
using OglBufferManagerPtr = OglBufferManager*;


// ==========================================================================
// OglVertexBuffer
//

class OglVertexBuffer :
	public RendererVertexBuffer
{
protected:
	OglVertexBuffer();

public:
	~OglVertexBuffer() override;
}; // OglVertexBuffer

using OglVertexBufferPtr = OglVertexBuffer*;
using OglVertexBufferUPtr = std::unique_ptr<OglVertexBuffer>;

//
// OglVertexBuffer
// ==========================================================================


// ==========================================================================
// OglVertexBufferFactory
//

struct OglVertexBufferFactory final
{
	static OglVertexBufferUPtr create(
		const OglBufferManagerPtr ogl_buffer_manager,
		const RendererVertexBufferCreateParam& param);
}; // OglVertexBufferFactory

//
// OglVertexBufferFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_VERTEX_BUFFER_INCLUDED
