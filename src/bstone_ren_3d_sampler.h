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
// 3D renderer's sampler.
//


#ifndef BSTONE_REN_3D_SAMPLER_INCLUDED
#define BSTONE_REN_3D_SAMPLER_INCLUDED


#include <memory>

#include "bstone_ren_3d_types.h"


namespace bstone
{


// ==========================================================================
// Ren3dSampler
//

struct Ren3dSamplerState
{
	Ren3dFilterKind mag_filter_;
	Ren3dFilterKind min_filter_;

	Ren3dMipmapMode mipmap_mode_;

	Ren3dAddressMode address_mode_u_;
	Ren3dAddressMode address_mode_v_;

	int anisotropy_;
}; // Ren3dSamplerState

struct Ren3dCreateSamplerParam
{
	Ren3dSamplerState state_;
}; // Ren3dCreateSamplerParam

struct Ren3dSamplerUpdateParam
{
	Ren3dSamplerState state_;
}; // Ren3dSamplerUpdateParam


class Ren3dSampler
{
protected:
	Ren3dSampler() = default;


public:
	virtual ~Ren3dSampler() = default;


	virtual void update(
		const Ren3dSamplerUpdateParam& param) = 0;

	virtual const Ren3dSamplerState& get_state() const noexcept = 0;
}; // Ren3dSampler

using Ren3dSamplerPtr = Ren3dSampler*;
using Ren3dSamplerUPtr = std::unique_ptr<Ren3dSampler>;

//
// Ren3dSampler
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_SAMPLER_INCLUDED
