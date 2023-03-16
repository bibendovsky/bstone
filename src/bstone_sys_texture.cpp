/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_texture.h"

namespace bstone {
namespace sys {

void Texture::set_blend_mode(TextureBlendMode mode)
try
{
	do_set_blend_mode(mode);
}
BSTONE_STATIC_THROW_NESTED_FUNC

void Texture::copy(const R2Rect* texture_rect, const R2Rect* target_rect)
try
{
	return do_copy(texture_rect, target_rect);
}
BSTONE_STATIC_THROW_NESTED_FUNC

TextureLockUPtr Texture::make_lock()
try
{
	return do_make_lock(nullptr);
}
BSTONE_STATIC_THROW_NESTED_FUNC

TextureLockUPtr Texture::make_lock(R2Rect rect)
try
{
	return do_make_lock(&rect);
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace sys
} // namespace bstone
