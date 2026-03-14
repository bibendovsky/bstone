/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Vertex input

#include "bstone_null_r3r_vertex_input.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rVertexInputImpl final : public R3rVertexInput
{
public:
	NullR3rVertexInputImpl(const R3rCreateVertexInputParam& param);
	~NullR3rVertexInputImpl() override = default;
};

// --------------------------------------------------------------------------

NullR3rVertexInputImpl::NullR3rVertexInputImpl([[maybe_unused]] const R3rCreateVertexInputParam& param)
{}

} // namespace

// ==========================================================================

NullR3rVertexInputUPtr make_null_r3r_vertex_input(const R3rCreateVertexInputParam& param)
{
	return std::make_unique<NullR3rVertexInputImpl>(param);
}

} // namespace bstone
