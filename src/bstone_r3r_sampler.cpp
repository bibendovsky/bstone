/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Sampler

#include "bstone_exception.h"
#include "bstone_r3r_sampler.h"

namespace bstone {

void R3rSampler::update(const R3rSamplerUpdateParam& param)
try
{
	do_update(param);
}
BSTONE_STATIC_THROW_NESTED_FUNC

const R3rSamplerState& R3rSampler::get_state() const noexcept
try
{
	return do_get_state();
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace bstone
