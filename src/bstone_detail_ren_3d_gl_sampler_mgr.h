/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// OpenGL sampler object manager (implementation interface).
//


#ifndef BSTONE_DETAIL_REN_3D_GL_SAMPLER_MGR_INCLUDED
#define BSTONE_DETAIL_REN_3D_GL_SAMPLER_MGR_INCLUDED


#include <memory>

#include "bstone_ren_3d_sampler.h"


namespace bstone
{
namespace detail
{


class Ren3dGlContext;
using Ren3dGlContextPtr = Ren3dGlContext*;


// ==========================================================================
// Ren3dGlSamplerMgr
//

class Ren3dGlSamplerMgr
{
public:
	Ren3dGlSamplerMgr() noexcept = default;

	virtual ~Ren3dGlSamplerMgr() = default;


	virtual Ren3dSamplerUPtr create(
		const Ren3dCreateSamplerParam& param) = 0;

	virtual void notify_destroy(
		const Ren3dSamplerPtr sampler) noexcept = 0;

	virtual void set(
		const Ren3dSamplerPtr sampler) = 0;

	virtual const Ren3dSamplerState& get_current_state() const noexcept = 0;
}; // Ren3dGlSamplerMgr

using Ren3dGlSamplerMgrPtr = Ren3dGlSamplerMgr*;
using Ren3dGlSamplerMgrUPtr = std::unique_ptr<Ren3dGlSamplerMgr>;

//
// Ren3dGlSamplerMgr
// ==========================================================================


// ==========================================================================
// Ren3dGlSamplerMgrFactory
//

struct Ren3dGlSamplerMgrFactory
{
	static Ren3dGlSamplerMgrUPtr create(
		const Ren3dGlContextPtr context);
}; // Ren3dGlSamplerMgrFactory

//
// Ren3dGlSamplerMgrFactory
// ==========================================================================


} // detail
} // bstone


#endif // !BSTONE_DETAIL_REN_3D_GL_SAMPLER_MGR_INCLUDED
