/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Fixed-size read-only memory stream.

#include <algorithm>
#include <memory>

#include "bstone_exception.h"
#include "bstone_static_ro_memory_stream.h"

namespace bstone {

StaticRoMemoryStream::StaticRoMemoryStream(const void* buffer, IntP size)
{
	open(buffer, size);
}

const UInt8* StaticRoMemoryStream::get_data() const
{
	ensure_is_open();

	return buffer_;
}

const UInt8* StaticRoMemoryStream::get_data()
{
	ensure_is_open();

	return buffer_;
}

void StaticRoMemoryStream::open(const void* buffer, IntP size)
{
	close_internal();

	is_open_ = true;
	buffer_ = static_cast<const UInt8*>(buffer);
	size_ = size;
}

void StaticRoMemoryStream::do_close()
{
	close_internal();
}

bool StaticRoMemoryStream::do_is_open() const
{
	return is_open_;
}

IntP StaticRoMemoryStream::do_read(void* buffer, IntP count)
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

IntP StaticRoMemoryStream::do_write(const void*, IntP)
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

Int64 StaticRoMemoryStream::do_seek(Int64 offset, StreamOrigin origin)
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

Int64 StaticRoMemoryStream::do_get_size() const
{
	ensure_is_open();

	return size_;
}

void StaticRoMemoryStream::do_set_size(Int64)
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void StaticRoMemoryStream::do_flush()
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void StaticRoMemoryStream::ensure_is_open() const
{
	if (!is_open_)
	{
		BSTONE_THROW_STATIC_SOURCE("Closed stream.");
	}
}

void StaticRoMemoryStream::close_internal() noexcept
{
	is_open_ = false;
	buffer_ = nullptr;
	position_ = 0;
	size_ = 0;
}

} // namespace bstone
