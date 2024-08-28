/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Renderer's texture.

#ifndef BSTONE_SYS_TEXTURE_INCLUDED
#define BSTONE_SYS_TEXTURE_INCLUDED

#include <memory>

#include "bstone_sys_pixel_format.h"
#include "bstone_sys_rectangle.h"
#include "bstone_sys_texture_lock.h"

namespace bstone {
namespace sys {

enum class TextureBlendMode
{
	none,
	blend,
};

// ==========================================================================

enum class TextureAccess
{
	none,
	streaming,
};

// ==========================================================================

class Texture
{
public:
	Texture();
	virtual ~Texture();

	void set_blend_mode(TextureBlendMode mode);
	void copy(const Rectangle* texture_rectangle, const Rectangle* target_rectangle);
	TextureLockUPtr make_lock();

private:
	virtual void do_set_blend_mode(TextureBlendMode mode) = 0;
	virtual void do_copy(const Rectangle* texture_rectangle, const Rectangle* target_rectangle) = 0;

	virtual TextureLockUPtr do_make_lock(const Rectangle* rectangle) = 0;
};

// ==========================================================================

struct TextureInitParam
{
	PixelFormat pixel_format;
	TextureAccess access;
	int width;
	int height;
};

// ==========================================================================

using TextureUPtr = std::unique_ptr<Texture>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_TEXTURE_INCLUDED
