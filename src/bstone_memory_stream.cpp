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

MemoryStream::MemoryStream(IntP initial_capacity, IntP chunk_size)
{
	open(initial_capacity, chunk_size);
}

const UInt8* MemoryStream::get_data() const
{
	ensure_is_open();

	return storage_.get();
}

UInt8* MemoryStream::get_data()
{
	ensure_is_open();

	return storage_.get();
}

void MemoryStream::open(IntP initial_capacity, IntP chunk_size)
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

IntP MemoryStream::do_read(void* buffer, IntP count)
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
	std::uninitialized_copy_n(storage_.get() + position_, copy_count, static_cast<UInt8*>(buffer));
	position_ += copy_count;

	return copy_count;
}

IntP MemoryStream::do_write(const void* buffer, IntP count)
{
	ensure_is_open();

	if (count == 0)
	{
		return 0;
	}

	const auto new_capacity = position_ + count;

	reserve(new_capacity, chunk_size_);
	std::uninitialized_copy_n(static_cast<const UInt8*>(buffer), count, storage_.get() + position_);
	position_ += count;
	size_ = new_capacity;

	return count;
}

Int64 MemoryStream::do_seek(Int64 offset, StreamOrigin origin)
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

Int64 MemoryStream::do_get_size() const
{
	ensure_is_open();

	return size_;
}

void MemoryStream::do_set_size(Int64 size)
{
	ensure_is_open();

	if (size > capacity_)
	{
		reserve(size, chunk_size_);

		if (position_ < size)
		{
			std::uninitialized_fill_n(storage_.get() + size_, size - position_, UInt8{});
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

void MemoryStream::reserve(IntP capacity, IntP chunk_size)
{
	if (capacity <= capacity_)
	{
		return;
	}

	const auto new_capacity = ((capacity + chunk_size - 1) / chunk_size) * chunk_size;
	auto storage = std::make_unique<UInt8[]>(static_cast<IntP>(new_capacity));

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
