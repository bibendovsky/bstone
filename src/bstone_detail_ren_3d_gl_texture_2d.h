/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
