/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_ALLOCATOR_BITMAP_INCLUDED)
#define BSTONE_ALLOCATOR_BITMAP_INCLUDED

#include <cstddef>
#include <array>
#include <stdexcept>
#include <type_traits>

#include "bstone_int.h"
#include "bstone_exception.h"

namespace bstone {

template<Int TMaxSize>
class AllocatorBitmap
{
	static_assert(TMaxSize > 0, "Invalid max size.");

public:
	using Index = decltype(TMaxSize);
	using Block = std::size_t;

public:
	static constexpr auto max_size = TMaxSize;
	static constexpr auto bits_per_block = static_cast<Index>(sizeof(Block)) * 8;
	static constexpr auto block_count = (max_size + bits_per_block - 1) / bits_per_block;
	static constexpr auto max_aligned_size = block_count * bits_per_block;
	static constexpr auto full_block = ~Block{};

public:
	AllocatorBitmap();

	bool is_empty() const noexcept;
	Index set_first_free();
	void reset(Index index);

private:
	using Bitmap = std::array<Block, block_count>;

	struct MultiplyMaxSizeTag {};
	struct NonMultiplyMaxSizeTag {};

private:
	Bitmap bitmap_{};
	Index size_{};

private:
	void ctor(MultiplyMaxSizeTag);
	void ctor(NonMultiplyMaxSizeTag);
};

// --------------------------------------------------------------------------

template<Int TBitCount>
AllocatorBitmap<TBitCount>::AllocatorBitmap()
{
	using Tag = std::conditional_t<
		max_aligned_size == max_size, MultiplyMaxSizeTag, NonMultiplyMaxSizeTag>;
	ctor(Tag{});
}

template<Int TBitCount>
bool AllocatorBitmap<TBitCount>::is_empty() const noexcept
{
	return size_ == 0;
}

template<Int TBitCount>
auto AllocatorBitmap<TBitCount>::set_first_free() -> Index
{
	if (size_ == max_size)
	{
		BSTONE_STATIC_THROW("No free bit.");
	}

	for (auto i_block = Index{}; i_block < max_aligned_size; ++i_block)
	{
		auto& block = bitmap_[i_block];

		if (block != full_block)
		{
			for (auto i_bit = Index{}; i_bit < bits_per_block; ++i_bit)
			{
				const auto mask = Block{1} << i_bit;

				if ((block & mask) == 0)
				{
					block |= mask;
					++size_;
					return (i_block * bits_per_block) + i_bit;
				}
			}
		}
	}

	BSTONE_STATIC_THROW("No free bit.");
}

template<Int TBitCount>
void AllocatorBitmap<TBitCount>::reset(Index index)
{
	if (index < 0 || index >= max_size)
	{
		BSTONE_STATIC_THROW("Index out of range.");
	}

	const auto block_index = index / bits_per_block;
	const auto bit_index = index % bits_per_block;
	const auto mask = Block{1} << bit_index;

	if ((bitmap_[block_index] & mask) == 0)
	{
		BSTONE_STATIC_THROW("Unset bit.");
	}

	bitmap_[block_index] &= full_block ^ mask;
	--size_;
}

template<Int TBitCount>
void AllocatorBitmap<TBitCount>::ctor(MultiplyMaxSizeTag)
{}

template<Int TBitCount>
void AllocatorBitmap<TBitCount>::ctor(NonMultiplyMaxSizeTag)
{
	constexpr auto unused_bit_count = max_aligned_size - max_size;
	constexpr auto remain_bit_count = bits_per_block - unused_bit_count;
	static_assert(remain_bit_count > 0 && remain_bit_count < bits_per_block, "Invalid remain bit count.");
	bitmap_.back() = full_block << remain_bit_count;
}

} // namespace bstone

#endif // BSTONE_ALLOCATOR_BITMAP_INCLUDED
