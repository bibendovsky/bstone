/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Tracks allocated blocks in memory pool.

#if !defined(BSTONE_MEMORY_POOL_BITMAP_INCLUDED)
#define BSTONE_MEMORY_POOL_BITMAP_INCLUDED

#include <memory>
#include <type_traits>

#include "bstone_array.h"
#include "bstone_exception.h"
#include "bstone_int.h"
#include "bstone_memory_resource.h"

namespace bstone {

template<IntP TSize>
class MemoryPoolBitmapStaticStorage
{
public:
	static constexpr bool is_dynamic() noexcept
	{
		return false;
	}

	static constexpr IntP get_size() noexcept
	{
		return TSize;
	}

	const bool& operator[](IntP index) const
	{
		return storage_[index];
	}

	bool& operator[](IntP index)
	{
		return storage_[index];
	}

	void reset()
	{
		storage_.fill(false);
	}

private:
	using Storage = Array<bool, get_size()>;

private:
	Storage storage_{};
};

// ==========================================================================

class MemoryPoolBitmapDynamicStorage
{
public:
	MemoryPoolBitmapDynamicStorage()
		:
		storage_{nullptr, StorageDeleter{}}
	{}

	MemoryPoolBitmapDynamicStorage(IntP size, MemoryResource& memory_resource)
	{
		resize(size, memory_resource);
	}

	static constexpr bool is_dynamic() noexcept
	{
		return true;
	}

	IntP get_size() const noexcept
	{
		return size_;
	}

	void reset()
	{
		storage_ = nullptr;
		size_ = 0;
	}

	void resize(IntP size, MemoryResource& memory_resource)
	{
		reset();

		auto storage = Storage{static_cast<bool*>(memory_resource.allocate(size)), StorageDeleter{memory_resource}};
		std::uninitialized_fill_n(storage.get(), size, false);
		storage_.swap(storage);
		size_ = size;
	}

	const bool& operator[](IntP index) const
	{
		return storage_[index];
	}

	bool& operator[](IntP index)
	{
		return storage_[index];
	}

private:
	class StorageDeleter
	{
	public:
		StorageDeleter()
			:
			memory_resource_{&get_null_memory_resource()}
		{}

		StorageDeleter(MemoryResource& memory_resource)
			:
			memory_resource_{&memory_resource}
		{}

		void operator()(bool* ptr) const
		{
			memory_resource_->deallocate(ptr);
		}

	private:
		MemoryResource* memory_resource_{};
	};

	using Storage = std::unique_ptr<bool[], StorageDeleter>;

private:
	Storage storage_{};
	IntP size_{};
};

// ==========================================================================

template<typename TStorage>
class MemoryPoolBitmap
{
public:
	using Storage = TStorage;

public:
	MemoryPoolBitmap() = default;

	template<typename UStorage = Storage, std::enable_if_t<UStorage::is_dynamic(), int> = 0>
	MemoryPoolBitmap(IntP size, MemoryResource& memory_resource)
	{
		resize(size, memory_resource);
	}

	template<typename UStorage = Storage, std::enable_if_t<UStorage::is_dynamic(), int> = 0>
	void resize(IntP size, MemoryResource& memory_resource)
	try {
		if (!is_empty())
		{
			BSTONE_THROW_STATIC_SOURCE("In use.");
		}

		bitmap_.resize(size, memory_resource);
	} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

	template<typename UStorage = Storage, std::enable_if_t<UStorage::is_dynamic(), int> = 0>
	void reset_storage()
	{
		bitmap_.reset();
	}

	IntP get_size() const noexcept;
	bool is_empty() const noexcept;
	IntP set_first_free();
	void reset(IntP index);

private:
	Storage bitmap_{};
	IntP size_{};
	IntP pivot_index_{};
};

// --------------------------------------------------------------------------

template<typename TStorage>
IntP MemoryPoolBitmap<TStorage>::get_size() const noexcept
{
	return size_;
}

template<typename TStorage>
bool MemoryPoolBitmap<TStorage>::is_empty() const noexcept
{
	return size_ == 0;
}

template<typename TStorage>
IntP MemoryPoolBitmap<TStorage>::set_first_free()
try
{
	constexpr auto no_free_bit_message = "No free bit.";

	const auto max_size = bitmap_.get_size();

	if (size_ == max_size)
	{
		BSTONE_THROW_STATIC_SOURCE(no_free_bit_message);
	}

	struct IndexPair
	{
		IntP from;
		IntP to;
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

	BSTONE_THROW_STATIC_SOURCE(no_free_bit_message);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

template<typename TStorage>
void MemoryPoolBitmap<TStorage>::reset(IntP index)
try
{
	bitmap_[index] = false;
	--size_;
	pivot_index_ = index;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

template<IntP TSize>
using StaticMemoryPoolBitmap = MemoryPoolBitmap<MemoryPoolBitmapStaticStorage<TSize>>;

using DynamicMemoryPoolBitmap = MemoryPoolBitmap<MemoryPoolBitmapDynamicStorage>;

} // namespace bstone

#endif // BSTONE_MEMORY_POOL_BITMAP_INCLUDED
