/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Stream primitive.

#include "bstone_stream.h"

#include "bstone_exception.h"

namespace bstone {

Stream::Stream() noexcept = default;

Stream::~Stream() = default;

void Stream::close() noexcept
{
	do_close();
}

bool Stream::is_open() const noexcept
{
	return do_is_open();
}

std::intptr_t Stream::read(void* buffer, std::intptr_t count)
{
	return do_read(buffer, count);
}

void Stream::read_exactly(void* buffer, std::intptr_t count)
{
	auto buffer_bytes = static_cast<char*>(buffer);

	while (true)
	{
		const auto read_count = read(buffer_bytes, count);

		if (read_count == 0)
		{
			if (count != 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Mismatch of read number of bytes.");
			}

			break;
		}

		buffer_bytes += read_count;
		count -= read_count;
	}
}

std::intptr_t Stream::write(const void* buffer, std::intptr_t count)
{
	return do_write(buffer, count);
}

void Stream::write_exactly(const void* buffer, std::intptr_t count)
{
	auto buffer_bytes = static_cast<const char*>(buffer);

	while (count != 0)
	{
		const auto written_count = write(buffer_bytes, count);

		if (written_count == 0)
		{
			if (count != 0)
			{
				BSTONE_THROW_STATIC_SOURCE("Mismatch of written number of bytes.");
			}

			break;
		}

		buffer_bytes += written_count;
		count -= written_count;
	}
}

std::int64_t Stream::seek(std::int64_t offset, StreamOrigin origin)
{
	return do_seek(offset, origin);
}

std::int64_t Stream::skip(std::int64_t offset)
{
	return seek(offset, StreamOrigin::current);
}

std::int64_t Stream::get_position()
{
	return skip(0);
}

void Stream::set_position(std::int64_t position)
{
	seek(position, StreamOrigin::begin);
}

std::int64_t Stream::get_size()
{
	return do_get_size();
}

void Stream::set_size(std::int64_t size)
{
	do_set_size(size);
}

void Stream::flush()
{
	do_flush();
}

} // namespace bstone
