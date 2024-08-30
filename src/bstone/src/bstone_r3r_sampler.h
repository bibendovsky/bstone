/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Sampler

#ifndef BSTONE_R3R_SAMPLER_INCLUDED
#define BSTONE_R3R_SAMPLER_INCLUDED

#include <memory>
#include "bstone_r3r_types.h"

namespace bstone {

struct R3rSamplerState
{
	R3rFilterType mag_filter;
	R3rFilterType min_filter;

	R3rMipmapMode mipmap_mode;

	R3rAddressMode address_mode_u;
	R3rAddressMode address_mode_v;

	int anisotropy;
};

// ==========================================================================

struct R3rSamplerInitParam
{
	R3rSamplerState state;
};

struct R3rSamplerUpdateParam
{
	R3rSamplerState state;
};

// ==========================================================================

class R3rSampler
{
public:
	R3rSampler() noexcept;
	virtual ~R3rSampler();

	void update(const R3rSamplerUpdateParam& param);
	const R3rSamplerState& get_state() const noexcept;

private:
	virtual void do_update(const R3rSamplerUpdateParam& param) = 0;
	virtual const R3rSamplerState& do_get_state() const noexcept = 0;
};

// ==========================================================================

using R3rSamplerUPtr = std::unique_ptr<R3rSampler>;

} // namespace bstone

#endif // BSTONE_R3R_SAMPLER_INCLUDED
