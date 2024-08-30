/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Vertex Input

#ifndef BSTONE_R3R_VERTEX_INPUT_INCLUDED
#define BSTONE_R3R_VERTEX_INPUT_INCLUDED

#include <memory>
#include <vector>
#include "bstone_r3r_buffer.h"
#include "bstone_r3r_types.h"

namespace bstone {

enum class R3rVertexAttribFormat
{
	none = 0,
	rgba_8_unorm,
	rg_32_sfloat,
	rgb_32_sfloat,
};

// ==========================================================================

struct R3rVertexAttribDescr
{
	bool is_default;
	int location;
	R3rVertexAttribFormat format;
	R3rBuffer* vertex_buffer;
	int offset;
	int stride;
	R3rVec4 default_value;
};

// ==========================================================================

using R3rVertexAttribDescrs = std::vector<R3rVertexAttribDescr>;

struct R3rCreateVertexInputParam
{
	R3rBuffer* index_buffer;
	R3rVertexAttribDescrs attrib_descrs;
};

// ==========================================================================

class R3rVertexInput
{
public:
	R3rVertexInput() noexcept;
	virtual ~R3rVertexInput();
};

// ==========================================================================

using R3rVertexInputUPtr = std::unique_ptr<R3rVertexInput>;

} // namespace bstone

#endif // BSTONE_R3R_VERTEX_INPUT_INCLUDED
