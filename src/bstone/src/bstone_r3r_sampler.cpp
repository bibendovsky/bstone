/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Sampler

#include "bstone_exception.h"
#include "bstone_r3r_sampler.h"

namespace bstone {

R3rSampler::R3rSampler() noexcept = default;

R3rSampler::~R3rSampler() = default;

void R3rSampler::update(const R3rSamplerUpdateParam& param)
try {
	do_update(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

const R3rSamplerState& R3rSampler::get_state() const noexcept
try {
	return do_get_state();
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
