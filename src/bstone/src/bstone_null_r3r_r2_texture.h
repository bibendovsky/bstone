/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: 2D texture

#ifndef BSTONE_NULL_R3R_R2_TEXTURE_INCLUDED
#define BSTONE_NULL_R3R_R2_TEXTURE_INCLUDED

#include <memory>
#include "bstone_r3r_r2_texture.h"
#include "bstone_r3r_sampler.h"

// ==========================================================================

namespace bstone {

using NullR3rR2TextureUPtr = std::unique_ptr<R3rR2Texture>;

NullR3rR2TextureUPtr make_null_r3r_r2_texture(const R3rR2TextureInitParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_R2_TEXTURE_INCLUDED
