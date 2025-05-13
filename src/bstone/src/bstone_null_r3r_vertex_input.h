/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Vertex input

#ifndef BSTONE_NULL_R3R_VERTEX_INPUT_INCLUDED
#define BSTONE_NULL_R3R_VERTEX_INPUT_INCLUDED

#include <memory>
#include "bstone_r3r_vertex_input.h"

// ==========================================================================

namespace bstone {

using NullR3rVertexInputUPtr = std::unique_ptr<R3rVertexInput>;

NullR3rVertexInputUPtr make_null_r3r_vertex_input(const R3rCreateVertexInputParam& param);

} // namespace bstone

#endif // BSTONE_NULL_R3R_VERTEX_INPUT_INCLUDED
