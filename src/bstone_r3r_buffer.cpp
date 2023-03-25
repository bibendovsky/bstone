/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// 3D Renderer: Buffer

#include "bstone_exception.h"
#include "bstone_r3r_buffer.h"

namespace bstone {

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
try
{
	do_update(param);
}
BSTONE_STATIC_THROW_NESTED_FUNC

} // namespace bstone
