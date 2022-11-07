/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SPAN_INCLUDED
#define BSTONE_SPAN_INCLUDED

#include <cassert>
#include "bstone_int.h"
#include "bstone_type_traits.h"
#include "bstone_utility.h"

namespace bstone {

template<typename T>
class Span
{
public:
	using Item = T;

	constexpr Span() noexcept = default;

	constexpr Span(Item* items, Int size)
		:
		items_{items},
		size_{size}
	{
		assert(is_empty() || (!is_empty() && has_data()));
	}

	template<Int TSize>
	constexpr explicit Span(Item (&array)[TSize]) noexcept
		:
		items_{array},
		size_{TSize}
	{}

	constexpr Span(const Span& rhs) noexcept = default;
	constexpr Span& operator=(const Span& rhs) noexcept = default;

	constexpr Item* get_data() const noexcept
	{
		return items_;
	}

	constexpr Int get_size() const noexcept
	{
		return size_;
	}

	constexpr bool has_data() const noexcept
	{
		return get_data() != nullptr;
	}

	constexpr bool is_empty() const noexcept
	{
		return get_size() == 0;
	}

	constexpr Int get_bytes_size() const noexcept
	{
		return get_size() * static_cast<Int>(sizeof(Item));
	}

	constexpr Item* begin() const noexcept
	{
		return get_data();
	}

	constexpr Item* end() const noexcept
	{
		return begin() + get_size();
	}

	constexpr const Item& get_front() const
	{
		assert(has_data());
		assert(!is_empty());
		return *(begin());
	}

	constexpr Item& get_front()
	{
		return const_cast<Item&>(type_traits::as_const(*this).get_front());
	}

	constexpr const Item& get_back() const
	{
		assert(has_data());
		assert(!is_empty());
		return *(end() - 1);
	}

	constexpr Item& get_back()
	{
		return const_cast<Item&>(type_traits::as_const(*this).get_back());
	}

	constexpr Span get_subspan(Int offset, Int size) const
	{
		assert(offset >= 0);
		assert(size >= 0);
		assert((offset + size) <= get_size());
		return Span{get_data() + offset, size};
	}

	constexpr Span get_subspan(Int offset) const
	{
		return get_subspan(offset, get_size() - offset);
	}

	template<
		typename UItem = Item,
		std::enable_if_t<std::is_same<std::remove_const_t<UItem>, UInt8>::value, int> = 0>
	Span to_bytes() const noexcept
	{
		return *this;
	}

	template<
		typename UItem = Item,
		std::enable_if_t<!std::is_same<std::remove_const_t<UItem>, UInt8>::value, int> = 0>
	auto to_bytes() const noexcept
	{
		using UInt8Type = std::conditional_t<std::is_const<Item>::value, const UInt8, UInt8>;
		using UInt8Ptr = std::conditional_t<std::is_const<Item>::value, const UInt8*, UInt8*>;
		return Span<UInt8Type>{reinterpret_cast<UInt8Ptr>(get_data()), get_bytes_size()};
	}

	template<typename UItem = Item, std::enable_if_t<std::is_const<UItem>::value, int> = 0>
	constexpr Span to_const() const noexcept
	{
		return *this;
	}

	template<typename UItem = Item, std::enable_if_t<!std::is_const<UItem>::value, int> = 0>
	constexpr Span<const UItem> to_const() const noexcept
	{
		return Span<const UItem>{get_data(), get_size()};
	}

	constexpr Item& operator[](Int index) const
	{
		assert(index >= 0 && index < get_size());
		assert(has_data());
		return get_data()[index];
	}

	constexpr void swap(Span& rhs) noexcept
	{
		utility::swap(items_, rhs.items_);
		utility::swap(size_, rhs.size_);
	}

private:
	Item* items_{};
	Int size_{};
}; // Span

// ==========================================================================

template<typename T>
inline constexpr auto make_span(T* items, Int size)
{
	return Span<T>{items, size};
}

template<typename T, Int TSize>
inline constexpr auto make_span(T (&array)[TSize]) noexcept
{
	return Span<T>{array};
}

// ==========================================================================

template<typename T>
inline constexpr auto make_bytes_span(T* items, Int size)
{
	using Type = std::remove_pointer_t<T>;
	using UInt8Type = std::conditional_t<std::is_const<Type>::value, const UInt8, UInt8>;
	using UInt8PtrType = std::conditional_t<std::is_const<Type>::value, const UInt8*, UInt8*>;

	return Span<UInt8Type>
	{
		reinterpret_cast<UInt8PtrType>(items),
		size * static_cast<Int>(sizeof(T))
	};
}

template<typename T, Int TSize>
inline constexpr auto make_bytes_span(T (&array)[TSize]) noexcept
{
	return make_bytes_span(array, TSize);
}

} // namespace bstone

#endif // !BSTONE_SPAN_INCLUDED
