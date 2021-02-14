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
// 3D renderer's vertex input.
//


#ifndef BSTONE_REN_3D_VERTEX_INPUT_INCLUDED
#define BSTONE_REN_3D_VERTEX_INPUT_INCLUDED


#include <memory>
#include <vector>

#include "bstone_ren_3d_buffer.h"
#include "bstone_ren_3d_types.h"


namespace bstone
{


// ==========================================================================
// Ren3dVertexInput
//

enum class Ren3dVertexAttribFormat
{
	none,
	rgba_8_unorm,
	rg_32_sfloat,
	rgb_32_sfloat,
}; // Ren3dVertexAttribFormat

struct Ren3dVertexAttribDescr
{
	bool is_default_;
	int location_;
	Ren3dVertexAttribFormat format_;
	Ren3dBufferPtr vertex_buffer_;
	int offset_;
	int stride_;
	Ren3dVec4 default_value_;
}; // Ren3dVertexAttribDescr

using Ren3dVertexAttribDescrs = std::vector<Ren3dVertexAttribDescr>;

struct Ren3dCreateVertexInputParam
{
	Ren3dBufferPtr index_buffer_;
	Ren3dVertexAttribDescrs attrib_descrs_;
}; // Ren3dCreateVertexInputParam


class Ren3dVertexInput
{
protected:
	Ren3dVertexInput() = default;


public:
	virtual ~Ren3dVertexInput() = default;
}; // Ren3dVertexInput

using Ren3dVertexInputPtr = Ren3dVertexInput*;
using Ren3dVertexInputUPtr = std::unique_ptr<Ren3dVertexInput>;

//
// Ren3dVertexInput
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_VERTEX_INPUT_INCLUDED
