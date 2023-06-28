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
BSTONE_BEGIN_FUNC_TRY
	do_set_blend_mode(mode);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Texture::copy(const R2RectI* texture_rect, const R2RectI* target_rect)
BSTONE_BEGIN_FUNC_TRY
	return do_copy(texture_rect, target_rect);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureLockUPtr Texture::make_lock()
BSTONE_BEGIN_FUNC_TRY
	return do_make_lock(nullptr);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

TextureLockUPtr Texture::make_lock(R2RectI rect)
BSTONE_BEGIN_FUNC_TRY
	return do_make_lock(&rect);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
