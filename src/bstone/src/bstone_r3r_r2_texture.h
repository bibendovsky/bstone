/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: 2D Texture

#ifndef BSTONE_R3R_R2_TEXTURE_INCLUDED
#define BSTONE_R3R_R2_TEXTURE_INCLUDED

#include <memory>
#include "bstone_r3r_types.h"

namespace bstone {

struct R3rR2TextureInitParam
{
	R3rPixelFormat pixel_format;

	int width;
	int height;

	int mipmap_count;
};

struct R3rR2TextureUpdateParam
{
	int mipmap_level;

	const void* image;
};

// ==========================================================================

class R3rR2Texture
{
public:
	R3rR2Texture() noexcept;
	virtual ~R3rR2Texture();

	void update(const R3rR2TextureUpdateParam& param);
	void generate_mipmaps();

private:
	virtual void do_update(const R3rR2TextureUpdateParam& param) = 0;
	virtual void do_generate_mipmaps() = 0;
};

// ==========================================================================

using R3rR2TextureUPtr = std::unique_ptr<R3rR2Texture>;

} // namespace bstone

#endif // BSTONE_R3R_R2_TEXTURE_INCLUDED
