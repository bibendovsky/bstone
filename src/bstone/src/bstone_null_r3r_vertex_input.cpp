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
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rVertexInputImpl final : public R3rVertexInput
{
public:
	NullR3rVertexInputImpl(const R3rCreateVertexInputParam& param);
	~NullR3rVertexInputImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	using MemoryPool = FixedPoolResource<NullR3rVertexInputImpl, R3rLimits::max_vertex_inputs()>;

private:
	static MemoryPool memory_pool_;
};

// --------------------------------------------------------------------------

NullR3rVertexInputImpl::MemoryPool NullR3rVertexInputImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rVertexInputImpl::NullR3rVertexInputImpl([[maybe_unused]] const R3rCreateVertexInputParam& param)
{}

void* NullR3rVertexInputImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rVertexInputImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

} // namespace

// ==========================================================================

NullR3rVertexInputUPtr make_null_r3r_vertex_input(const R3rCreateVertexInputParam& param)
{
	return std::make_unique<NullR3rVertexInputImpl>(param);
}

} // namespace bstone
