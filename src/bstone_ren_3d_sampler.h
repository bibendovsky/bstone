/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
public:
	Ren3dSampler() noexcept = default;

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
