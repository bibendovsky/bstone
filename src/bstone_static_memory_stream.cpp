/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Writable memory stream with external fixed-capacity storage.

#include "bstone_static_memory_stream.h"

#include <algorithm>
#include <memory>

#include "bstone_exception.h"

namespace bstone {

StaticMemoryStream::StaticMemoryStream(void* buffer, std::intptr_t size)
{
	open(buffer, size);
}

const std::uint8_t* StaticMemoryStream::get_data() const
{
	ensure_is_open();

	return buffer_;
}

std::uint8_t* StaticMemoryStream::get_data()
{
	ensure_is_open();

	return buffer_;
}

void StaticMemoryStream::open(void* buffer, std::intptr_t buffer_size)
{
	close_internal();
	validate_buffer(buffer);
	validate_buffer_size(buffer_size);

	is_open_ = true;
	buffer_ = static_cast<std::uint8_t*>(buffer);
	capacity_ = buffer_size;
	size_ = 0;
}

void StaticMemoryStream::do_close() noexcept
{
	close_internal();
}

bool StaticMemoryStream::do_is_open() const noexcept
{
	return is_open_;
}

std::intptr_t StaticMemoryStream::do_read(void* buffer, std::intptr_t count)
{
	ensure_is_open();
	validate_buffer(buffer);
	validate_count(count);

	const auto copy_count = std::min(count, size_ - position_);

	if (copy_count <= 0)
	{
		return 0;
	}

	std::uninitialized_copy_n(&buffer_[position_], copy_count, static_cast<std::uint8_t*>(buffer));
	position_ += copy_count;

	return copy_count;
}

std::intptr_t StaticMemoryStream::do_write(const void* buffer, std::intptr_t count)
{
	ensure_is_open();
	validate_buffer(buffer);
	validate_count(count);

	const auto copy_count = std::min(count, capacity_ - position_);

	if (copy_count <= 0)
	{
		return 0;
	}

	std::uninitialized_copy_n(static_cast<const std::uint8_t*>(buffer), copy_count, &buffer_[position_]);
	position_ += copy_count;
	size_ = position_;

	return copy_count;
}

std::int64_t StaticMemoryStream::do_seek(std::int64_t offset, StreamOrigin origin)
{
	ensure_is_open();

	auto new_position = std::int64_t{};

	switch (origin)
	{
		case StreamOrigin::begin:
			new_position = offset;
			break;

		case StreamOrigin::current:
			if (INTPTR_MAX - position_ < offset)
			{
				BSTONE_THROW_STATIC_SOURCE("New position out of range.");
			}

			new_position = position_ + offset;
			break;

		case StreamOrigin::end:
			if (INTPTR_MAX - size_ < offset)
			{
				BSTONE_THROW_STATIC_SOURCE("New position out of range.");
			}

			new_position = size_ + offset;
			break;

		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	if (new_position < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative new position.");
	}

	position_ = static_cast<std::intptr_t>(new_position);

	return position_;
}

std::int64_t StaticMemoryStream::do_get_size()
{
	ensure_is_open();

	return size_;
}

void StaticMemoryStream::do_set_size(std::int64_t size)
{
	ensure_is_open();
	validate_size(size);

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
		BSTONE_THROW_STATIC_SOURCE("Closed.");
	}
}

void StaticMemoryStream::validate_buffer(const void* buffer)
{
	if (buffer == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null buffer.");
	}
}

void StaticMemoryStream::validate_buffer_size(std::intptr_t buffer_size)
{
	if (buffer_size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative buffer size.");
	}
}

void StaticMemoryStream::validate_count(std::intptr_t count)
{
	if (count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative count.");
	}
}

void StaticMemoryStream::validate_size(std::int64_t size)
{
	if (size < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Negative size.");
	}

	if (size > INTPTR_MAX)
	{
		BSTONE_THROW_STATIC_SOURCE("Size out of range.");
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
