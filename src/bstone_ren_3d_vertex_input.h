/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
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
public:
	Ren3dVertexInput() noexcept = default;

	virtual ~Ren3dVertexInput() = default;
}; // Ren3dVertexInput

using Ren3dVertexInputPtr = Ren3dVertexInput*;
using Ren3dVertexInputUPtr = std::unique_ptr<Ren3dVertexInput>;

//
// Ren3dVertexInput
// ==========================================================================


} // bstone


#endif // !BSTONE_REN_3D_VERTEX_INPUT_INCLUDED
