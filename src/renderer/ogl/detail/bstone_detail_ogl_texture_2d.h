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
// OpenGL 2D texture (implementation).
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_OGL_TEXTURE_2D_INCLUDED
#define BSTONE_DETAIL_OGL_TEXTURE_2D_INCLUDED


#include "bstone_renderer.h"


namespace bstone
{
namespace detail
{


class OglTextureManager;
using OglTextureManagerPtr = OglTextureManager*;


// =========================================================================
// OglTexture2d
//

class OglTexture2d :
	public RendererTexture2d
{
protected:
	OglTexture2d();


public:
	~OglTexture2d() override;


	virtual void bind() = 0;

	virtual void update_sampler_state(
		const RendererSamplerState& new_sampler_state) = 0;
}; // OglTexture2d

using OglTexture2dPtr = OglTexture2d*;
using OglTexture2dUPtr = std::unique_ptr<OglTexture2d>;

//
// OglTexture2d
// =========================================================================


// =========================================================================
// OglTexture2dFactory
//

struct OglTexture2dFactory final
{
	static OglTexture2dUPtr create(
		const OglTextureManagerPtr ogl_texture_manager,
		const RendererTexture2dCreateParam& param);
}; // OglTexture2dFactory

//
// OglTexture2dFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_OGL_TEXTURE_2D_INCLUDED
