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


#include "bstone_memory_stream.h"

#include <algorithm>
#include <memory>


namespace bstone
{


MemoryStream::MemoryStream(
	const int initial_capacity,
	const StreamOpenMode open_mode)
	:
	is_open_{},
	is_readable_{},
	is_writable_{},
	position_{},
	size_{},
	ext_size_{},
	buffer_{},
	ext_buffer_{},
	int_buffer_{}
{
	static_cast<void>(open(initial_capacity, open_mode));
}

MemoryStream::MemoryStream(
	const int buffer_size,
	const int buffer_offset,
	const std::uint8_t* buffer,
	const StreamOpenMode open_mode)
	:
	is_open_{},
	is_readable_{},
	is_writable_{},
	position_{},
	size_{},
	ext_size_{},
	buffer_{},
	ext_buffer_{},
	int_buffer_{}
{
	static_cast<void>(open(buffer_size, buffer_offset, buffer, open_mode));
}

MemoryStream::MemoryStream(
	MemoryStream&& rhs)
	:
	is_open_{std::move(rhs.is_open_)},
	is_readable_{std::move(rhs.is_readable_)},
	is_writable_{std::move(rhs.is_writable_)},
	position_{std::move(rhs.position_)},
	size_{std::move(rhs.size_)},
	ext_size_{std::move(rhs.ext_size_)},
	buffer_{std::move(rhs.buffer_)},
	ext_buffer_{std::move(rhs.ext_buffer_)},
	int_buffer_{std::move(rhs.int_buffer_)}
{
	rhs.is_open_ = false;
}

MemoryStream::~MemoryStream()
{
	close_internal();
}

bool MemoryStream::open(
	const int initial_capacity,
	const StreamOpenMode open_mode)
{
	close_internal();


	auto is_readable = false;
	auto is_writable = false;

	switch (open_mode)
	{
	case StreamOpenMode::read:
		is_readable = true;
		break;

	case StreamOpenMode::write:
		is_writable = true;
		break;

	case StreamOpenMode::read_write:
		is_readable = true;
		is_writable = true;
		break;

	default:
		return false;
	}


	auto capacity = initial_capacity;

	if (capacity < 0)
	{
		capacity = 0;
	}

	int_buffer_.reserve(capacity);

	is_open_ = true;
	is_readable_ = is_readable;
	is_writable_ = is_writable;

	return true;
}

bool MemoryStream::open(
	const int buffer_size,
	const int buffer_offset,
	const std::uint8_t* buffer,
	const StreamOpenMode open_mode)
{
	close_internal();

	if (buffer_size < 0)
	{
		return false;
	}

	if (!buffer)
	{
		return false;
	}


	auto is_readable = false;
	auto is_writable = false;

	switch (open_mode)
	{
	case StreamOpenMode::read:
		is_readable = true;
		break;

	case StreamOpenMode::write:
		is_writable = true;
		break;

	case StreamOpenMode::read_write:
		is_readable = true;
		is_writable = true;
		break;

	default:
		return false;
	}


	is_open_ = true;
	is_readable_ = is_readable;
	is_writable_ = is_writable;
	size_ = buffer_size;
	ext_size_ = buffer_size;
	buffer_ = const_cast<std::uint8_t*>(&buffer[buffer_offset]);
	ext_buffer_ = buffer_;

	return true;
}

void MemoryStream::close()
{
	close_internal();
}

bool MemoryStream::is_open() const
{
	return is_open_;
}

std::int64_t MemoryStream::get_size()
{
	return size_;
}

bool MemoryStream::set_size(
	const std::int64_t size)
{
	if (!is_open_)
	{
		return false;
	}

	if (!is_writable_)
	{
		return false;
	}

	if (size < 0)
	{
		return false;
	}

	if (ext_buffer_)
	{
		return false;
	}

	int_buffer_.resize(static_cast<std::size_t>(size));

	size_ = size;

	if (size_ > 0)
	{
		buffer_ = reinterpret_cast<std::uint8_t*>(&int_buffer_[0]);
	}
	else
	{
		buffer_ = nullptr;
	}

	return true;
}

std::int64_t MemoryStream::seek(
	const std::int64_t offset,
	const StreamSeekOrigin origin)
{
	if (!is_open_)
	{
		return -1;
	}

	switch (origin)
	{
	case StreamSeekOrigin::begin:
		position_ = offset;
		break;

	case StreamSeekOrigin::current:
		position_ += offset;
		break;

	case StreamSeekOrigin::end:
		position_ = size_ + offset;
		break;

	default:
		return -1;
	}

	if (position_ < 0)
	{
		position_ = 0;
	}

	return position_;
}

std::int64_t MemoryStream::get_position()
{
	return position_;
}

int MemoryStream::read(
	void* buffer,
	const int count)
{
	if (!is_open_)
	{
		return 0;
	}

	if (!is_readable_)
	{
		return 0;
	}

	if (!buffer)
	{
		return 0;
	}

	if (count <= 0)
	{
		return 0;
	}

	const auto remain = size_ - position_;

	if (remain <= 0)
	{
		return 0;
	}

	auto read_count = static_cast<int>(std::min(static_cast<std::int64_t>(count), remain));

	std::uninitialized_copy_n(&buffer_[position_], read_count, static_cast<std::uint8_t*>(buffer));

	position_ += read_count;

	return read_count;
}

bool MemoryStream::write(
	const void* buffer,
	const int count)
{
	if (!is_open_)
	{
		return false;
	}

	if (!is_writable_)
	{
		return false;
	}

	if (count < 0)
	{
		return false;
	}

	if (count == 0)
	{
		return true;
	}

	if (!buffer)
	{
		return false;
	}

	if (!ext_buffer_)
	{
		auto new_size = position_ + count;

		if (new_size > size_)
		{
			int_buffer_.resize(static_cast<std::size_t>(new_size));

			size_ = new_size;
			buffer_ = reinterpret_cast<std::uint8_t*>(&int_buffer_[0]);
		}
	}
	else
	{
		if ((position_ + count) > ext_size_)
		{
			return false;
		}
	}

	std::uninitialized_copy_n(static_cast<const std::uint8_t*>(buffer), count, &buffer_[position_]);

	position_ += count;

	return true;
}

bool MemoryStream::flush()
{
	return is_open_;
}

bool MemoryStream::is_readable() const
{
	return is_open_ && is_readable_;
}

bool MemoryStream::is_seekable() const
{
	return is_open_;
}

bool MemoryStream::is_writable() const
{
	return is_open_ && is_writable_;
}

std::uint8_t* MemoryStream::get_data()
{
	return buffer_;
}

const std::uint8_t* MemoryStream::get_data() const
{
	return buffer_;
}

bool MemoryStream::remove_block(
	const std::int64_t offset,
	const int count)
{
	if (!is_open_)
	{
		return false;
	}

	if (offset < 0)
	{
		return false;
	}

	if (count < 0)
	{
		return false;
	}

	if (count == 0)
	{
		return true;
	}

	if ((offset + count) > size_)
	{
		return false;
	}

	const auto where_it = int_buffer_.begin() + static_cast<std::intptr_t>(offset);

	int_buffer_.erase(where_it, where_it + count);

	size_ -= count;

	return true;
}

void MemoryStream::close_internal()
{
	is_open_ = false;
	is_readable_ = false;
	is_writable_ = false;
	position_ = 0;
	size_ = 0;
	ext_size_ = 0;
	buffer_ = nullptr;
	ext_buffer_ = nullptr;
	int_buffer_ = {};
}


} // bstone
