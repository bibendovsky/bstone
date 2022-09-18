/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
