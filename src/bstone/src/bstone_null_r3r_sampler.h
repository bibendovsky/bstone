/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Sampler

#ifndef BSTONE_NULL_R3R_SAMPLER_INCLUDED
#define BSTONE_NULL_R3R_SAMPLER_INCLUDED

#include "bstone_r3r_sampler.h"

// ==========================================================================

namespace bstone {

using NullR3rSamplerUPtr = std::unique_ptr<R3rSampler>;

NullR3rSamplerUPtr make_null_r3r_sampler(const R3rSamplerInitParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_SAMPLER_INCLUDED
