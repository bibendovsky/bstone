/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#include "bstone_exception.h"
#include "bstone_file.h"
#include "bstone_utility.h"

namespace bstone {

void File::close() noexcept
{
	resource_.reset();
}

BSTONE_CXX_NODISCARD bool File::is_open() const noexcept
{
	return !resource_.is_empty();
}

void File::read_exact(void* buffer, std::intptr_t count) const
{
	if (read(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data underflow.");
	}
}

void File::write_exact(const void* buffer, std::intptr_t count) const
{
	if (write(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data overflow.");
	}
}

std::int64_t File::skip(std::int64_t delta) const
{
	return seek(delta, FileOrigin::current);
}

BSTONE_CXX_NODISCARD std::int64_t File::get_position() const
{
	return skip(0);
}

void File::set_position(std::int64_t position) const
{
	seek(position, FileOrigin::begin);
}

} // namespace bstone
