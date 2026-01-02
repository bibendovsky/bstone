/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Renderer's texture

#ifndef BSTONE_SYS_TEXTURE_INCLUDED
#define BSTONE_SYS_TEXTURE_INCLUDED

#include "bstone_sys_pixel_format.h"
#include "bstone_sys_rectangle.h"
#include "bstone_sys_texture_lock.h"
#include <memory>

namespace bstone::sys {

enum class TextureBlendMode
{
	none,
	blend,
};

enum class TextureAccess
{
	none,
	streaming,
};

struct TextureInitParam
{
	PixelFormat pixel_format;
	TextureAccess access;
	int width;
	int height;
};

// ======================================

class Texture
{
public:
	Texture() = default;
	virtual ~Texture() = default;

	void set_blend_mode(TextureBlendMode mode);
	void copy(const Rectangle* texture_rectangle, const Rectangle* target_rectangle);
	TextureLockUPtr make_lock();

private:
	virtual void do_set_blend_mode(TextureBlendMode mode) = 0;
	virtual void do_copy(const Rectangle* texture_rectangle, const Rectangle* target_rectangle) = 0;
	virtual TextureLockUPtr do_make_lock(const Rectangle* rectangle) = 0;
};

// ======================================

using TextureUPtr = std::unique_ptr<Texture>;

} // namespace bstone::sys

#endif // BSTONE_SYS_TEXTURE_INCLUDED
