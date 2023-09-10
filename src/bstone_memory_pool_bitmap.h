/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Tracks allocated blocks in memory pool.

#if !defined(BSTONE_MEMORY_POOL_BITMAP_INCLUDED)
#define BSTONE_MEMORY_POOL_BITMAP_INCLUDED

#include <array>

#include "bstone_int.h"
#include "bstone_exception.h"

namespace bstone {

using MemoryPoolBitmapInt = IntP;

// ==========================================================================

template<MemoryPoolBitmapInt TMaxSize>
class MemoryPoolBitmap
{
public:
	static constexpr auto max_size = TMaxSize;

public:
	bool is_empty() const noexcept;
	MemoryPoolBitmapInt set_first_free();
	void reset(MemoryPoolBitmapInt index);

private:
	using Bitmap = std::array<bool, max_size>;

private:
	Bitmap bitmap_{};
	MemoryPoolBitmapInt size_{};
	MemoryPoolBitmapInt pivot_index_{};
};

// --------------------------------------------------------------------------

template<MemoryPoolBitmapInt TMaxSize>
bool MemoryPoolBitmap<TMaxSize>::is_empty() const noexcept
{
	return size_ == 0;
}

template<MemoryPoolBitmapInt TMaxSize>
MemoryPoolBitmapInt MemoryPoolBitmap<TMaxSize>::set_first_free()
try
{
	if (size_ == max_size)
	{
		BSTONE_THROW_STATIC_SOURCE("No free bit.");
	}

	struct IndexPair
	{
		MemoryPoolBitmapInt from;
		MemoryPoolBitmapInt to;
	};

	const IndexPair index_pairs[2] = {{pivot_index_, max_size}, {0, pivot_index_}};

	for (const auto& index_pair : index_pairs)
	{
		for (auto i = index_pair.from; i < index_pair.to; ++i)
		{
			if (!bitmap_[i])
			{
				bitmap_[i] = true;
				++size_;
				pivot_index_ = i;
				return i;
			}
		}
	}

	BSTONE_THROW_STATIC_SOURCE("No free bit.");
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<MemoryPoolBitmapInt TMaxSize>
void MemoryPoolBitmap<TMaxSize>::reset(MemoryPoolBitmapInt index)
try
{
	bitmap_[index] = false;
	--size_;
	pivot_index_ = index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

#endif // BSTONE_MEMORY_POOL_BITMAP_INCLUDED
