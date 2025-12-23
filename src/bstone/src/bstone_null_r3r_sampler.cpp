/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Sampler

#include "bstone_null_r3r_sampler.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_fixed_pool_resource.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rSamplerImpl final : public R3rSampler
{
public:
	NullR3rSamplerImpl(const R3rSamplerInitParam& param);
	~NullR3rSamplerImpl() override {}

	void* operator new(size_t size);
	void operator delete(void* ptr);

private:
	void do_update(const R3rSamplerUpdateParam& param) override;

	const R3rSamplerState& do_get_state() const noexcept override;

private:
	using MemoryPool = FixedPoolResource<NullR3rSamplerImpl, R3rLimits::max_samplers()>;

private:
	static MemoryPool memory_pool_;

private:
	R3rSamplerState state_{};
};

// --------------------------------------------------------------------------

NullR3rSamplerImpl::MemoryPool NullR3rSamplerImpl::memory_pool_{};

// --------------------------------------------------------------------------

NullR3rSamplerImpl::NullR3rSamplerImpl(const R3rSamplerInitParam& param)
	:
	state_{param.state}
{}

void* NullR3rSamplerImpl::operator new(size_t size)
try {
	return memory_pool_.allocate(static_cast<intptr_t>(size));
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullR3rSamplerImpl::operator delete(void* ptr)
{
	memory_pool_.deallocate(ptr);
}

void NullR3rSamplerImpl::do_update([[maybe_unused]] const R3rSamplerUpdateParam& param)
{}

const R3rSamplerState& NullR3rSamplerImpl::do_get_state() const noexcept
{
	return state_;
}

} // namespace

// ==========================================================================

NullR3rSamplerUPtr make_null_r3r_sampler(const R3rSamplerInitParam& param)
{
	return std::make_unique<NullR3rSamplerImpl>(param);
}

} // namespace bstone
