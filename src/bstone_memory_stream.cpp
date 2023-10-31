/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Resizable memory stream.

#include <algorithm>

#include "bstone_exception.h"
#include "bstone_memory_stream.h"

namespace bstone {

MemoryStream::MemoryStream(std::intptr_t initial_capacity, std::intptr_t chunk_size)
{
	open(initial_capacity, chunk_size);
}

const std::uint8_t* MemoryStream::get_data() const
{
	ensure_is_open();

	return storage_.get();
}

std::uint8_t* MemoryStream::get_data()
{
	ensure_is_open();

	return storage_.get();
}

void MemoryStream::open(std::intptr_t initial_capacity, std::intptr_t chunk_size)
{
	close_internal();
	reserve(initial_capacity, chunk_size);
	is_open_ = true;
	chunk_size_ = chunk_size;
}

void MemoryStream::do_close()
{
	close_internal();
}

bool MemoryStream::do_is_open() const
{
	return is_open_;
}

std::intptr_t MemoryStream::do_read(void* buffer, std::intptr_t count)
{
	ensure_is_open();

	if (count == 0)
	{
		return 0;
	}

	const auto remain_size = size_ - position_;

	if (remain_size <= 0)
	{
		return 0;
	}

	const auto copy_count = std::min(count, remain_size);
	std::uninitialized_copy_n(storage_.get() + position_, copy_count, static_cast<std::uint8_t*>(buffer));
	position_ += copy_count;

	return copy_count;
}

std::intptr_t MemoryStream::do_write(const void* buffer, std::intptr_t count)
{
	ensure_is_open();

	if (count == 0)
	{
		return 0;
	}

	const auto new_capacity = position_ + count;

	reserve(new_capacity, chunk_size_);
	std::uninitialized_copy_n(static_cast<const std::uint8_t*>(buffer), count, storage_.get() + position_);
	position_ += count;
	size_ = new_capacity;

	return count;
}

std::int64_t MemoryStream::do_seek(std::int64_t offset, StreamOrigin origin)
{
	ensure_is_open();

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

std::int64_t MemoryStream::do_get_size() const
{
	ensure_is_open();

	return size_;
}

void MemoryStream::do_set_size(std::int64_t size)
{
	ensure_is_open();

	if (size > capacity_)
	{
		reserve(size, chunk_size_);

		if (position_ < size)
		{
			std::uninitialized_fill_n(storage_.get() + size_, size - position_, std::uint8_t{});
		}
	}

	size_ = size;
}

void MemoryStream::do_flush()
{
	ensure_is_open();
}

void MemoryStream::ensure_is_open() const
{
	if (!is_open_)
	{
		BSTONE_THROW_STATIC_SOURCE("Closed stream.");
	}
}

void MemoryStream::reserve(std::intptr_t capacity, std::intptr_t chunk_size)
{
	if (capacity <= capacity_)
	{
		return;
	}

	const auto new_capacity = ((capacity + chunk_size - 1) / chunk_size) * chunk_size;
	auto storage = std::make_unique<std::uint8_t[]>(static_cast<std::intptr_t>(new_capacity));

	if (storage_ != nullptr)
	{
		std::uninitialized_copy_n(storage_.get(), size_, storage.get());
	}

	capacity_ = new_capacity;
	storage_.swap(storage);
}

void MemoryStream::close_internal()
{
	is_open_ = false;
	size_ = 0;
	position_ = 0;
}

} // namespace bstone
