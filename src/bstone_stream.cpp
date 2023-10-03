/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Stream primitive.

#include "bstone_exception.h"
#include "bstone_stream.h"

namespace bstone {

void Stream::close()
{
	do_close();
}

bool Stream::is_open() const
{
	return do_is_open();
}

IntP Stream::read(void* buffer, IntP count)
{
	return do_read(buffer, count);
}

void Stream::read_exact(void* buffer, IntP count)
{
	if (read(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data underflow.");
	}
}

IntP Stream::write(const void* buffer, IntP count)
{
	return do_write(buffer, count);
}

void Stream::write_exact(const void* buffer, IntP count)
{
	if (write(buffer, count) != count)
	{
		BSTONE_THROW_STATIC_SOURCE("Data overflow.");
	}
}

Int64 Stream::seek(Int64 offset, StreamOrigin origin)
{
	return do_seek(offset, origin);
}

Int64 Stream::skip(Int64 delta)
{
	return seek(delta, StreamOrigin::current);
}

Int64 Stream::get_position()
{
	return skip(0);
}

void Stream::set_position(Int64 position)
{
	seek(position, StreamOrigin::begin);
}

Int64 Stream::get_size() const
{
	return do_get_size();
}

void Stream::set_size(Int64 size)
{
	do_set_size(size);
}

void Stream::flush()
{
	do_flush();
}

} // namespace bstone
