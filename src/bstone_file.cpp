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

void File::close()
{
	resource_.reset();
}

bool File::is_open() const
{
	return !resource_.is_empty();
}

void File::read_exact(void* buffer, IntP count) const
{
	if (read(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data underflow.");
	}
}

void File::write_exact(const void* buffer, IntP count) const
{
	if (write(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data overflow.");
	}
}

Int64 File::skip(Int64 delta) const
{
	return seek(delta, FileOrigin::current);
}

Int64 File::get_position() const
{
	return skip(0);
}

void File::set_position(Int64 position) const
{
	seek(position, FileOrigin::begin);
}

} // namespace bstone
