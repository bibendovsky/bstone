/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// 3D renderer's 2D texture.
//


#ifndef BSTONE_REN_3D_TEXTURE_2D_INCLUDED
#define BSTONE_REN_3D_TEXTURE_2D_INCLUDED


#include <memory>

#include "bstone_ren_3d_types.h"


namespace bstone
{


// ==========================================================================
// Ren3dTexture2d
//

struct Ren3dCreateTexture2dParam
{
	Ren3dPixelFormat pixel_format_;

	int width_;
	int height_;

	int mipmap_count_;
}; // Ren3dCreateTexture2dParam

struct Ren3dTexture2dUpdateParam
{
	int mipmap_level_;

	const void* image_;
}; // Ren3dTexture2dUpdateParam


class Ren3dTexture2d
{
public:
	Ren3dTexture2d() noexcept = default;

	virtual ~Ren3dTexture2d() = default;


	virtual void update(
		const Ren3dTexture2dUpdateParam& param) = 0;

	virtual void generate_mipmaps() = 0;
}; // Ren3dTexture2d

using Ren3dTexture2dPtr = Ren3dTexture2d*;
using Ren3dTexture2dUPtr = std::unique_ptr<Ren3dTexture2d>;

//
// Ren3dTexture2d
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_TEXTURE_2D_INCLUDED
