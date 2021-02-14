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
protected:
	Ren3dGlSamplerMgr() = default;


public:
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
