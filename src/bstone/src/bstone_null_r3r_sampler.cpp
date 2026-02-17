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
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rSamplerImpl final : public R3rSampler
{
public:
	NullR3rSamplerImpl(const R3rSamplerInitParam& param);
	~NullR3rSamplerImpl() override {}

	void update(const R3rSamplerUpdateParam& param) override;
	const R3rSamplerState& get_state() const override;

private:
	R3rSamplerState state_{};
};

// --------------------------------------------------------------------------

NullR3rSamplerImpl::NullR3rSamplerImpl(const R3rSamplerInitParam& param)
	:
	state_{param.state}
{}

void NullR3rSamplerImpl::update([[maybe_unused]] const R3rSamplerUpdateParam& param)
{}

const R3rSamplerState& NullR3rSamplerImpl::get_state() const
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
