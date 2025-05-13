/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Shader

#include "bstone_null_r3r_shader.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rShaderImpl final : public R3rShader
{
public:
	NullR3rShaderImpl(const R3rShaderInitParam& param);
	~NullR3rShaderImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	R3rShaderType do_get_type() const noexcept override;

private:
	using MemoryPool = FixedPoolResource<NullR3rShaderImpl, R3rLimits::max_shaders()>;

private:
	static MemoryPool memory_pool_;

private:
	R3rShaderType type_{};
};

// --------------------------------------------------------------------------

NullR3rShaderImpl::MemoryPool NullR3rShaderImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rShaderImpl::NullR3rShaderImpl(const R3rShaderInitParam& param)
	:
	type_(param.type)
{}

void* NullR3rShaderImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rShaderImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

R3rShaderType NullR3rShaderImpl::do_get_type() const noexcept
{
	return type_;
}

} // namespace

// ==========================================================================

NullR3rShaderUPtr make_null_r3r_shader(const R3rShaderInitParam& param)
{
	return std::make_unique<NullR3rShaderImpl>(param);
}

} // namespace bstone
