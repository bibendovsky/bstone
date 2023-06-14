/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_memory_stream.h"
#include <algorithm>
#include <cstdint>
#include <memory>
#include <utility>

namespace bstone
{

MemoryStream::MemoryStream(int initial_capacity, StreamOpenMode open_mode) noexcept
{
	static_cast<void>(open(initial_capacity, open_mode));
}

MemoryStream::MemoryStream(
	int buffer_size,
	int buffer_offset,
	const unsigned char* buffer,
	StreamOpenMode open_mode) noexcept
{
	static_cast<void>(open(buffer_size, buffer_offset, buffer, open_mode));
}

MemoryStream::MemoryStream(MemoryStream&& rhs) noexcept
{
	std::swap(is_open_, rhs.is_open_);
	std::swap(is_readable_, rhs.is_readable_);
	std::swap(is_writable_, rhs.is_writable_);
	std::swap(position_, rhs.position_);
	std::swap(size_, rhs.size_);
	std::swap(ext_size_, rhs.ext_size_);
	std::swap(buffer_, rhs.buffer_);
	std::swap(ext_buffer_, rhs.ext_buffer_);
	int_buffer_.swap(rhs.int_buffer_);
}

MemoryStream::~MemoryStream()
{
	close_internal();
}

bool MemoryStream::open(int initial_capacity, StreamOpenMode open_mode) noexcept
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
	int buffer_size,
	int buffer_offset,
	const unsigned char* buffer,
	StreamOpenMode open_mode) noexcept
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
	buffer_ = const_cast<unsigned char*>(&buffer[buffer_offset]);
	ext_buffer_ = buffer_;
	return true;
}

void MemoryStream::close() noexcept
{
	close_internal();
}

bool MemoryStream::is_open() const noexcept
{
	return is_open_;
}

int MemoryStream::get_size() noexcept
{
	return size_;
}

bool MemoryStream::set_size(int size) noexcept
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
		buffer_ = reinterpret_cast<unsigned char*>(&int_buffer_[0]);
	}
	else
	{
		buffer_ = nullptr;
	}

	return true;
}

int MemoryStream::seek(int offset, StreamSeekOrigin origin) noexcept
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

int MemoryStream::get_position() noexcept
{
	return position_;
}

int MemoryStream::read(void* buffer, int count) noexcept
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

	auto read_count = static_cast<int>(std::min(count, remain));
	std::uninitialized_copy_n(&buffer_[position_], read_count, static_cast<unsigned char*>(buffer));
	position_ += read_count;
	return read_count;
}

bool MemoryStream::write(const void* buffer, int count) noexcept
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
			buffer_ = reinterpret_cast<unsigned char*>(&int_buffer_[0]);
		}
	}
	else
	{
		if ((position_ + count) > ext_size_)
		{
			return false;
		}
	}

	std::uninitialized_copy_n(static_cast<const unsigned char*>(buffer), count, &buffer_[position_]);
	position_ += count;
	return true;
}

bool MemoryStream::flush() noexcept
{
	return is_open_;
}

bool MemoryStream::is_readable() const noexcept
{
	return is_open_ && is_readable_;
}

bool MemoryStream::is_seekable() const noexcept
{
	return is_open_;
}

bool MemoryStream::is_writable() const noexcept
{
	return is_open_ && is_writable_;
}

std::uint8_t* MemoryStream::get_data() noexcept
{
	return buffer_;
}

const unsigned char* MemoryStream::get_data() const noexcept
{
	return buffer_;
}

bool MemoryStream::remove_block(int offset, int count) noexcept
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

void MemoryStream::close_internal() noexcept
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
