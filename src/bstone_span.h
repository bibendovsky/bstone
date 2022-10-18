/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SPAN_INCLUDED
#define BSTONE_SPAN_INCLUDED

#include <cassert>
#include <type_traits>
#include "bstone_int.h"
#include "bstone_type_traits.h"

namespace bstone {

template<typename T>
class Span
{
public:
	using Item = T;

	static constexpr auto item_size = static_cast<Int>(sizeof(Item));

	constexpr Span() noexcept = default;

	constexpr Span(Item* items, Int size) noexcept
		:
		data_{items},
		size_{size}
	{
		assert(is_empty() || (!is_empty() && has_data()));
	}

	template<Int USize>
	constexpr Span(Item (&array)[USize]) noexcept
		:
		Span{array, USize}
	{}

	constexpr Span(const Span&) noexcept = default;
	constexpr Span& operator=(const Span&) noexcept = default;

	constexpr Item* get_data() const noexcept
	{
		return data_;
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
		return item_size * get_size();
	}

	constexpr Item* begin() const noexcept
	{
		return get_data();
	}

	constexpr Item* end() const noexcept
	{
		return begin() + get_size();
	}

	constexpr const Item& get_front() const noexcept
	{
		assert(has_data());
		assert(!is_empty());
		return *(begin());
	}

	constexpr Item& get_front() noexcept
	{
		return const_cast<Item&>(as_const(*this).get_front());
	}

	constexpr const Item& get_back() const noexcept
	{
		assert(has_data());
		assert(!is_empty());
		return *(end() - 1);
	}

	constexpr Item& get_back() noexcept
	{
		return const_cast<Item&>(as_const(*this).get_back());
	}

	constexpr Span get_subspan(Int offset, Int size) const noexcept
	{
		assert(offset >= 0);
		assert(size >= 0);
		assert((offset + size) <= get_size());
		return Span{get_data() + offset, size};
	}

	constexpr Span get_subspan(Int offset) const noexcept
	{
		return get_subspan(offset, get_size() - offset);
	}

	template<typename UItem = Item, std::enable_if_t<!std::is_const<UItem>::value, int> = 0>
	constexpr operator Span<const UItem>() const noexcept
	{
		return Span<const UItem>{get_data(), get_size()};
	}

	constexpr Item& operator[](Int index) const noexcept
	{
		assert(has_data());
		assert(index >= 0 && index < get_size());
		return get_data()[index];
	}

private:
	Item* data_{};
	Int size_{};
}; // Span

// ==========================================================================

namespace detail {

template<typename T>
struct SpanToBytes
{
	using ItemType = std::conditional_t<std::is_const<T>::value, const UInt8, UInt8>;
	using SpanType = Span<ItemType>;
};

} // namespace detail

template<typename T>
inline auto as_bytes(Span<T> span) noexcept
{
	return typename detail::SpanToBytes<T>::SpanType
	{
		reinterpret_cast<typename detail::SpanToBytes<T>::ItemType*>(
			span.get_data()),
			span.get_bytes_size()
	};
}

// ==========================================================================

template<typename T, Int TSize>
inline constexpr auto make_span(T (&array)[TSize]) noexcept
{
	return Span<T>{array};
}

template<typename T>
inline constexpr auto make_span(T* data, Int size) noexcept
{
	return Span<T>{data, size};
}

// ==========================================================================

template<typename T, Int TSize>
inline constexpr auto make_bytes_span(T (&array)[TSize]) noexcept
{
	return as_bytes(Span<T>{array});
}

template<typename T>
inline constexpr auto make_bytes_span(T* data, Int size) noexcept
{
	return as_bytes(Span<T>{data, size});
}

} // namespace bstone

#endif // !BSTONE_SPAN_INCLUDED
