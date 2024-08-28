/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Renderer's texture.

#include "bstone_sys_texture.h"

namespace bstone {
namespace sys {

Texture::Texture() = default;

Texture::~Texture() = default;

void Texture::set_blend_mode(TextureBlendMode mode)
{
	do_set_blend_mode(mode);
}

void Texture::copy(const Rectangle* texture_rectangle, const Rectangle* target_rectangle)
{
	return do_copy(texture_rectangle, target_rectangle);
}

TextureLockUPtr Texture::make_lock()
{
	return do_make_lock(nullptr);
}

} // namespace sys
} // namespace bstone
