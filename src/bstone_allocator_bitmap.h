/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_ALLOCATOR_BITMAP_INCLUDED)
#define BSTONE_ALLOCATOR_BITMAP_INCLUDED

#include <cassert>
#include <cstddef>
#include "bstone_exception.h"

namespace bstone {

template<std::size_t TSize>
class AllocatorBitmap
{
public:
	static constexpr auto max_size = TSize;
	static constexpr auto bits_per_block = sizeof(std::size_t) * 8;
	static constexpr auto block_count = (max_size + bits_per_block - 1) / bits_per_block;
	static constexpr auto unused_bit_count = (block_count * bits_per_block) - max_size;

public:
	AllocatorBitmap();
	AllocatorBitmap(const AllocatorBitmap&) = delete;
	AllocatorBitmap& operator=(const AllocatorBitmap&) = delete;
	~AllocatorBitmap();

	bool is_empty() const noexcept;

	std::size_t set_first_free();
	void reset(std::size_t index);

private:
	using Bitmap = std::size_t[block_count];

	Bitmap bitmap_;
	std::size_t size_{};

	void initialize() noexcept;
};

// ==========================================================================

template<std::size_t TSize>
AllocatorBitmap<TSize>::AllocatorBitmap()
{
	initialize();
}

template<std::size_t TSize>
AllocatorBitmap<TSize>::~AllocatorBitmap()
{
	assert(size_ == 0);
}

template<std::size_t TSize>
bool AllocatorBitmap<TSize>::is_empty() const noexcept
{
	return size_ == 0;
}

template<std::size_t TSize>
std::size_t AllocatorBitmap<TSize>::set_first_free()
try
{
	if (size_ == max_size)
	{
		BSTONE_STATIC_THROW("Out of bits.");
	}

	for (auto i = std::size_t{}; i < block_count; ++i)
	{
		auto value = bitmap_[i];

		if (value == std::size_t{})
		{
			// Full block.
			continue;
		}

		for (auto j = std::size_t{}; j < bits_per_block; ++j)
		{
			if ((value & 1) != 0)
			{
				++size_;
				bitmap_[i] ^= std::size_t{1} << j;
				return (i * block_count) + j;
			}

			value >>= 1;
		}
	}

	BSTONE_STATIC_THROW("Out of bits.");
}
BSTONE_STATIC_THROW_NESTED_FUNC

template<std::size_t TSize>
void AllocatorBitmap<TSize>::reset(std::size_t index)
try
{
	if (index > max_size)
	{
		BSTONE_STATIC_THROW("Index out of range.");
	}

	if (is_empty())
	{
		BSTONE_STATIC_THROW("Empty bitmap.");
	}

	const auto block_index = index / bits_per_block;
	const auto bit_index = index - (block_index * bits_per_block);
	auto& value = bitmap_[block_index];
	const auto mask = std::size_t{1} << bit_index;
	const auto is_clear = (value & mask) != 0;

	if (is_clear)
	{
		BSTONE_STATIC_THROW("Already reseted.");
	}

	value |= mask;
	--size_;
}
BSTONE_STATIC_THROW_NESTED_FUNC

template<std::size_t TSize>
void AllocatorBitmap<TSize>::initialize() noexcept
{
	for (auto& block : bitmap_)
	{
		block = ~std::size_t{};
	}

	bitmap_[block_count - 1] >>= unused_bit_count;
}

} // namespace bstone

#endif // BSTONE_ALLOCATOR_BITMAP_INCLUDED
