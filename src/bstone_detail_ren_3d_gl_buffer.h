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
// OpenGL buffer object (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_BUFFER_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_BUFFER_INCLUDED


#include "bstone_ren_3d_buffer.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;


// =========================================================================
// Ren3dGlBuffer
//

class Ren3dGlBuffer :
	public Ren3dBuffer
{
protected:
	Ren3dGlBuffer() = default;


public:
	~Ren3dGlBuffer() override = default;


	virtual void set(
		const bool is_set) = 0;
}; // Ren3dGlBuffer


using Ren3dGlBufferPtr = Ren3dGlBuffer*;
using Ren3dGlBufferUPtr = std::unique_ptr<Ren3dGlBuffer>;

//
// Ren3dGlBuffer
// =========================================================================


// =========================================================================
// Ren3dGlBufferFactory
//

struct Ren3dGlBufferFactory
{
	static Ren3dGlBufferUPtr create(
		const Ren3dGlContextPtr context,
		const Ren3dCreateBufferParam& param);
}; // Ren3dGlBufferFactory

//
// Ren3dGlBufferFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_BUFFER_INCLUDED
