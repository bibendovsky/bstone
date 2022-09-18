/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL sampler.
//
// !!! Internal usage only !!!
//


#ifndef BSTONE_DETAIL_REN_3D_GL_SAMPLER_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SAMPLER_INCLUDED


#include "bstone_ren_3d_sampler.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;


// =========================================================================
// Ren3dGlSampler
//

class Ren3dGlSampler :
	public Ren3dSampler
{
protected:
	Ren3dGlSampler() = default;


public:
	~Ren3dGlSampler() override = default;


	virtual void set() = 0;
}; // Ren3dGlSampler

using Ren3dGlSamplerPtr = Ren3dGlSampler*;
using Ren3dGlSamplerUPtr = std::unique_ptr<Ren3dGlSampler>;

//
// Ren3dGlSampler
// =========================================================================


// =========================================================================
// Ren3dGlSamplerFactory
//

struct Ren3dGlSamplerFactory
{
	static Ren3dGlSamplerUPtr create(
		Ren3dGlContextPtr context,
		const Ren3dCreateSamplerParam& param);
}; // Ren3dGlSamplerFactory

//
// Ren3dGlSamplerFactory
// =========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SAMPLER_INCLUDED
