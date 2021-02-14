/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_stream.h"

#include <vector>

#include "bstone_un_value.h"


namespace bstone {


Stream::Stream()
{
}

Stream::~Stream()
{
}

void Stream::close()
{
}

std::int64_t Stream::skip(
	const int count)
{
	return seek(count, StreamSeekOrigin::current);
}

std::int64_t Stream::get_position()
{
	return seek(0, StreamSeekOrigin::current);
}

bool Stream::set_position(
	const std::int64_t position)
{
	return seek(position, StreamSeekOrigin::begin) >= 0;
}

int Stream::read_octet()
{
	std::uint8_t value;

	if (read(&value, 1) > 0)
	{
		return value;
	}
	else
	{
		return 0;
	}
}

bool Stream::write_octet(
	const std::uint8_t value)
{
	return write(&value, 1);
}

bool Stream::write_string(
	const std::string& string)
{
	return write(string.c_str(), static_cast<int>(string.length()));
}

bool Stream::copy_to(
	Stream* dst_stream,
	const int buffer_size)
{
	if (!dst_stream)
	{
		return false;
	}

	if (!dst_stream->is_writable())
	{
		return false;
	}

	auto new_buffer_size = buffer_size;

	if (new_buffer_size <= 0)
	{
		new_buffer_size = get_default_copy_buffer_size();
	}

	std::vector<UnValue<char>> buffer(new_buffer_size);

	for (int count = -1; count != 0; )
	{
		count = read(&buffer[0], new_buffer_size);

		if (!dst_stream->write(&buffer[0], count))
		{
			return false;
		}
	}

	return true;
}

int Stream::get_default_copy_buffer_size()
{
	return 4096;
}


} // bstone
