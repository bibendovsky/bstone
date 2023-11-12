/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Fixed-size read-only memory stream.

#include <algorithm>
#include <memory>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_static_ro_memory_stream.h"

namespace bstone {

StaticRoMemoryStream::StaticRoMemoryStream(const void* buffer, std::intptr_t size) noexcept
{
	open(buffer, size);
}

BSTONE_CXX_NODISCARD const std::uint8_t* StaticRoMemoryStream::get_data() const noexcept
{
	BSTONE_ASSERT(is_open());

	return buffer_;
}

BSTONE_CXX_NODISCARD const std::uint8_t* StaticRoMemoryStream::get_data() noexcept
{
	BSTONE_ASSERT(is_open());

	return buffer_;
}

void StaticRoMemoryStream::open(const void* buffer, std::intptr_t size) noexcept
{
	close_internal();

	is_open_ = true;
	buffer_ = static_cast<const std::uint8_t*>(buffer);
	size_ = size;
}

void StaticRoMemoryStream::do_close() noexcept
{
	close_internal();
}

BSTONE_CXX_NODISCARD bool StaticRoMemoryStream::do_is_open() const noexcept
{
	return is_open_;
}

std::intptr_t StaticRoMemoryStream::do_read(void* buffer, std::intptr_t count)
{
	BSTONE_ASSERT(is_open());

	const auto remain_size = size_ - position_;

	if (remain_size <= 0)
	{
		return 0;
	}

	const auto copy_count = std::min(count, remain_size);
	std::uninitialized_copy_n(buffer_ + position_, copy_count, static_cast<std::uint8_t*>(buffer));
	position_ += copy_count;
	return copy_count;
}

std::intptr_t StaticRoMemoryStream::do_write(const void*, std::intptr_t)
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

std::int64_t StaticRoMemoryStream::do_seek(std::int64_t offset, StreamOrigin origin)
{
	BSTONE_ASSERT(is_open());

	auto new_position = std::intptr_t{};

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

BSTONE_CXX_NODISCARD std::int64_t StaticRoMemoryStream::do_get_size() const
{
	BSTONE_ASSERT(is_open());

	return size_;
}

void StaticRoMemoryStream::do_set_size(std::int64_t)
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void StaticRoMemoryStream::do_flush()
{
	BSTONE_THROW_STATIC_SOURCE("Not supported.");
}

void StaticRoMemoryStream::close_internal() noexcept
{
	is_open_ = false;
	buffer_ = nullptr;
	position_ = 0;
	size_ = 0;
}

} // namespace bstone
