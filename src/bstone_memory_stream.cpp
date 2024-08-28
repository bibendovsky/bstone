/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Resizable memory stream.

#include "bstone_memory_stream.h"

#include <algorithm>

#include "bstone_assert.h"
#include "bstone_exception.h"

namespace bstone {

MemoryStream::MemoryStream() noexcept = default;

MemoryStream::MemoryStream(std::intptr_t capacity, std::intptr_t chunk_size)
{
	open(capacity, chunk_size);
}

MemoryStream::MemoryStream(MemoryStream&&) noexcept = default;

MemoryStream& MemoryStream::operator=(MemoryStream&&) noexcept = default;

MemoryStream::~MemoryStream() = default;

const std::uint8_t* MemoryStream::get_data() const
{
	BSTONE_ASSERT(is_open_);

	return storage_.get();
}

std::uint8_t* MemoryStream::get_data()
{
	BSTONE_ASSERT(is_open_);

	return storage_.get();
}

void MemoryStream::open(std::intptr_t capacity, std::intptr_t chunk_size)
{
	BSTONE_ASSERT(capacity > 0);
	BSTONE_ASSERT(chunk_size > 0);

	close_internal();
	reserve(capacity, chunk_size);
	is_open_ = true;
	chunk_size_ = chunk_size;
}

void MemoryStream::do_close() noexcept
{
	close_internal();
}

bool MemoryStream::do_is_open() const noexcept
{
	return is_open_;
}

std::intptr_t MemoryStream::do_read(void* buffer, std::intptr_t count)
{
	BSTONE_ASSERT(is_open_);
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);

	const auto copy_count = std::min(count, size_ - position_);

	if (copy_count <= 0)
	{
		return 0;
	}

	std::copy_n(&storage_[position_], copy_count, static_cast<std::uint8_t*>(buffer));
	position_ += copy_count;

	return copy_count;
}

std::intptr_t MemoryStream::do_write(const void* buffer, std::intptr_t count)
{
	BSTONE_ASSERT(is_open_);
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);

	const auto copy_count = std::min(count, INTPTR_MAX - position_);

	if (copy_count == 0)
	{
		return 0;
	}

	const auto new_capacity = position_ + copy_count;
	reserve(new_capacity, chunk_size_);
	std::copy_n(static_cast<const std::uint8_t*>(buffer), copy_count, &storage_[position_]);
	position_ += copy_count;
	size_ = std::max(size_, position_);

	return copy_count;
}

std::int64_t MemoryStream::do_seek(std::int64_t offset, StreamOrigin origin)
{
	BSTONE_ASSERT(is_open_);

	auto new_position = std::int64_t{};

	switch (origin)
	{
		case StreamOrigin::begin:
			new_position = offset;
			break;

		case StreamOrigin::current:
			if (offset > 0 && INTPTR_MAX - position_ < offset)
			{
				BSTONE_THROW_STATIC_SOURCE("New position out of range.");
			}

			new_position = position_ + offset;
			break;

		case StreamOrigin::end:
			if (offset > 0 && INTPTR_MAX - size_ < offset)
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

std::int64_t MemoryStream::do_get_size()
{
	BSTONE_ASSERT(is_open_);

	return size_;
}

void MemoryStream::do_set_size(std::int64_t size)
{
	BSTONE_ASSERT(is_open_);
	BSTONE_ASSERT(size >= 0);

	if (size > INTPTR_MAX)
	{
		BSTONE_THROW_STATIC_SOURCE("Size out of range.");
	}

	const auto size_intptr = static_cast<std::intptr_t>(size);

	if (size_intptr > capacity_)
	{
		reserve(size_intptr, chunk_size_);
	}

	size_ = size_intptr;
}

void MemoryStream::do_flush()
{
	BSTONE_ASSERT(is_open_);
}

void MemoryStream::reserve(std::intptr_t capacity, std::intptr_t chunk_size)
{
	if (capacity <= capacity_)
	{
		return;
	}

	if (INTPTR_MAX - capacity < chunk_size - 1)
	{
		BSTONE_THROW_STATIC_SOURCE("Out of memory.");
	}

	const auto new_capacity = ((capacity + chunk_size - 1) / chunk_size) * chunk_size;
	auto storage = std::make_unique<std::uint8_t[]>(static_cast<std::intptr_t>(new_capacity));

	if (storage_ != nullptr)
	{
		std::copy_n(storage_.get(), size_, storage.get());
	}

	capacity_ = new_capacity;
	storage_.swap(storage);
}

void MemoryStream::close_internal() noexcept
{
	is_open_ = false;
	size_ = 0;
	position_ = 0;
}

} // namespace bstone
