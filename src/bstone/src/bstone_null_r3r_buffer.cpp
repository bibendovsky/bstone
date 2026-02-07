/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2025 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Null 3D renderer: Buffer

#include "bstone_null_r3r_buffer.h"
#include <stddef.h>
#include <stdint.h>
#include "bstone_exception.h"
#include "bstone_r3r_limits.h"

// ==========================================================================

namespace bstone {

namespace {

class NullR3rBufferImpl final : public R3rBuffer
{
public:
	NullR3rBufferImpl(const R3rBufferInitParam& param);
	~NullR3rBufferImpl() override {}

private:
	R3rBufferType do_get_type() const override;
	R3rBufferUsageType do_get_usage_type() const override;
	int do_get_size() const override;

	void do_update(const R3rUpdateBufferParam& param) override;

private:
	R3rBufferType type_{};
	R3rBufferUsageType usage_type_{};
	int size_{};
};

// --------------------------------------------------------------------------

NullR3rBufferImpl::NullR3rBufferImpl(const R3rBufferInitParam& param)
	:
	type_{param.type},
	usage_type_{param.usage_type},
	size_{param.size}
{}

R3rBufferType NullR3rBufferImpl::do_get_type() const
{
	return type_;
}

R3rBufferUsageType NullR3rBufferImpl::do_get_usage_type() const
{
	return usage_type_;
}

int NullR3rBufferImpl::do_get_size() const
{
	return size_;
}

void NullR3rBufferImpl::do_update([[maybe_unused]] const R3rUpdateBufferParam& param)
{}

} // namespace

// ==========================================================================

NullR3rBufferUPtr make_null_r3r_buffer(const R3rBufferInitParam& param)
try {
	return std::make_unique<NullR3rBufferImpl>(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
