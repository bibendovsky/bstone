/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// OpenGL texture object manager (implementation interface).
//


#ifndef BSTONE_DETAIL_REN_3D_GL_TEXTURE_MGR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_TEXTURE_MGR_INCLUDED


#include <memory>

#include "bstone_ren_3d_sampler.h"
#include "bstone_ren_3d_texture_2d.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;

class Ren3dGlTexture2d;
using Ren3dGlTexture2dPtr = Ren3dGlTexture2d*;


// ==========================================================================
// Ren3dGlTextureMgr
//

class Ren3dGlTextureMgr
{
public:
	Ren3dGlTextureMgr() noexcept = default;

	virtual ~Ren3dGlTextureMgr() = default;


	virtual Ren3dGlContextPtr get_context() const noexcept = 0;


	virtual Ren3dTexture2dUPtr create(
		const Ren3dCreateTexture2dParam& param) = 0;


	virtual void set(
		const Ren3dGlTexture2dPtr texture_2d) = 0;
}; // Ren3dGlTextureMgr

using Ren3dGlTextureMgrPtr = Ren3dGlTextureMgr*;
using Ren3dGlTextureMgrUPtr = std::unique_ptr<Ren3dGlTextureMgr>;

//
// Ren3dGlTextureMgr
// ==========================================================================


// ==========================================================================
// Ren3dGlTextureMgrFactory
//

struct Ren3dGlTextureMgrFactory
{
	static Ren3dGlTextureMgrUPtr create(
		const Ren3dGlContextPtr context);
}; // Ren3dGlTextureMgrFactory

//
// Ren3dGlTextureMgrFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_TEXTURE_MGR_INCLUDED
