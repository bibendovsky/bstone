/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non-resizeable read-write memory stream.

#include <algorithm>
#include <memory>

#include "bstone_exception.h"
#include "bstone_static_memory_stream.h"

namespace bstone {

StaticMemoryStream::StaticMemoryStream(void* buffer, IntP size)
{
	open(buffer, size);
}

const UInt8* StaticMemoryStream::get_data() const
{
	ensure_is_open();

	return buffer_;
}

UInt8* StaticMemoryStream::get_data()
{
	ensure_is_open();

	return buffer_;
}

void StaticMemoryStream::open(void* buffer, IntP size)
{
	close_internal();

	is_open_ = true;
	buffer_ = static_cast<UInt8*>(buffer);
	capacity_ = size;
	size_ = 0;
}

void StaticMemoryStream::do_close()
{
	close_internal();
}

bool StaticMemoryStream::do_is_open() const
{
	return is_open_;
}

IntP StaticMemoryStream::do_read(void* buffer, IntP count)
{
	ensure_is_open();

	const auto remain_size = size_ - position_;

	if (remain_size <= 0)
	{
		return 0;
	}

	const auto copy_count = std::min(count, remain_size);
	std::uninitialized_copy_n(buffer_ + position_, copy_count, static_cast<UInt8*>(buffer));
	position_ += copy_count;
	return copy_count;
}

IntP StaticMemoryStream::do_write(const void* buffer, IntP count)
{
	ensure_is_open();

	if (position_ + count > capacity_)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of free space.");
	}

	const auto copy_count = std::min(count, capacity_ - position_);
	std::uninitialized_copy_n(static_cast<const UInt8*>(buffer), copy_count, buffer_ + position_);
	position_ += copy_count;
	size_ = position_;

	return copy_count;
}

Int64 StaticMemoryStream::do_seek(Int64 offset, StreamOrigin origin)
{
	ensure_is_open();

	auto new_position = IntP{};

	switch (origin)
	{
		case StreamOrigin::begin: new_position = offset; break;
		case StreamOrigin::current: new_position = position_ + offset; break;
		case StreamOrigin::end: new_position = size_ + offset; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	if (new_position < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative new position.");
	}

	position_ = new_position;

	return new_position;
}

Int64 StaticMemoryStream::do_get_size() const
{
	ensure_is_open();

	return size_;
}

void StaticMemoryStream::do_set_size(Int64 size)
{
	ensure_is_open();

	if (size > capacity_)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of free space.");
	}

	size_ = size;
}

void StaticMemoryStream::do_flush()
{
	ensure_is_open();
}

void StaticMemoryStream::ensure_is_open() const
{
	if (!is_open_)
	{
		BSTONE_THROW_STATIC_SOURCE("Closed stream.");
	}
}

void StaticMemoryStream::close_internal() noexcept
{
	is_open_ = false;
	buffer_ = nullptr;
	capacity_ = 0;
	position_ = 0;
	size_ = 0;
}

} // namespace bstone
