/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
protected:
	Ren3dTexture2d() = default;


public:
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
