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
// OpenGL 2D texture (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_TEXTURE_2D_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_TEXTURE_2D_INCLUDED


#include "bstone_ren_3d_sampler.h"
#include "bstone_ren_3d_texture_2d.h"


namespace bstone
{
namespace detail
{


class Ren3dGlTextureMgr;
using Ren3dGlTextureMgrPtr = Ren3dGlTextureMgr*;


// =========================================================================
// Ren3dGlTexture2d
//

class Ren3dGlTexture2d :
	public Ren3dTexture2d
{
protected:
	Ren3dGlTexture2d() = default;


public:
	~Ren3dGlTexture2d() override = default;


	virtual void set() = 0;

	virtual void bind() = 0;

	virtual void update_sampler_state(
		const Ren3dSamplerState& new_sampler_state) = 0;
}; // Ren3dGlTexture2d

using Ren3dGlTexture2dPtr = Ren3dGlTexture2d*;
using Ren3dGlTexture2dUPtr = std::unique_ptr<Ren3dGlTexture2d>;

//
// Ren3dGlTexture2d
// =========================================================================


// =========================================================================
// Ren3dGlTexture2dFactory
//

struct Ren3dGlTexture2dFactory
{
	static Ren3dGlTexture2dUPtr create(
		const Ren3dGlTextureMgrPtr texture_manager,
		const Ren3dCreateTexture2dParam& param);
}; // Ren3dGlTexture2dFactory

//
// Ren3dGlTexture2dFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_TEXTURE_2D_INCLUDED
