/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Buffer

#include "bstone_exception.h"
#include "bstone_r3r_buffer.h"

namespace bstone {

R3rBuffer::R3rBuffer() noexcept = default;

R3rBuffer::~R3rBuffer() = default;

R3rBufferType R3rBuffer::get_type() const noexcept
{
	return do_get_type();
}

R3rBufferUsageType R3rBuffer::get_usage_type() const noexcept
{
	return do_get_usage_type();
}

int R3rBuffer::get_size() const noexcept
{
	return do_get_size();
}

void R3rBuffer::update(const R3rUpdateBufferParam& param)
try {
	do_update(param);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone
