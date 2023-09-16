/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A contiguous sequence of objects.

#if !defined(BSTONE_SPAN_INCLUDED)
#define BSTONE_SPAN_INCLUDED

#include <cassert>

#include <type_traits>

#include "bstone_int.h"
#include "bstone_utility.h"

namespace bstone {

using SpanInt = IntP;

// ==========================================================================

template<typename T>
class Span
{
public:
	using Item = T;

public:
	Span() = default;

	constexpr Span(Item* items, SpanInt size)
		:
		items_{items},
		size_{size}
	{
		assert(items == nullptr && size == 0 || items != nullptr && size >= 0);
	}

	template<SpanInt TSize>
	constexpr explicit Span(Item (&items)[TSize]) noexcept
		:
		items_{items},
		size_{TSize}
	{}

	constexpr Item* get_data() const noexcept
	{
		return items_;
	}

	constexpr SpanInt get_size() const noexcept
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

	constexpr SpanInt get_bytes_size() const noexcept
	{
		return get_size() * static_cast<SpanInt>(sizeof(Item));
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
		return const_cast<Item&>(bstone::as_const(*this).get_front());
	}

	constexpr const Item& get_back() const
	{
		assert(has_data());
		assert(!is_empty());

		return *(end() - 1);
	}

	constexpr Item& get_back()
	{
		return const_cast<Item&>(bstone::as_const(*this).get_back());
	}

	constexpr Span get_subspan(SpanInt offset, SpanInt size) const
	{
		assert(offset >= 0);
		assert(size >= 0);
		assert((offset + size) <= get_size());

		return Span{get_data() + offset, size};
	}

	constexpr Span get_subspan(SpanInt offset) const
	{
		return get_subspan(offset, get_size() - offset);
	}

	constexpr Item& operator[](SpanInt index) const
	{
		assert(index >= 0 && index < get_size());
		assert(has_data());

		return get_data()[index];
	}

	constexpr void swap(Span& rhs) noexcept
	{
		bstone::swop(items_, rhs.items_);
		bstone::swop(size_, rhs.size_);
	}

private:
	Item* items_{};
	SpanInt size_{};
};

// ==========================================================================

template<typename T>
inline constexpr auto make_span(T* items, SpanInt size)
{
	return Span<T>{items, size};
}

template<typename T, SpanInt TSize>
inline constexpr auto make_span(T (&array)[TSize]) noexcept
{
	return Span<T>{array};
}

// ==========================================================================

template<typename T>
inline constexpr auto make_const_span(T* items, SpanInt size)
{
	return Span<const T>{items, size};
}

template<typename T, SpanInt TSize>
inline constexpr auto make_const_span(T (&array)[TSize]) noexcept
{
	return Span<const T>{array};
}

template<typename T>
inline auto make_const_span(Span<T> span) noexcept
{
	return Span<std::add_const_t<T>>{span.get_data(), span.get_size()};
}

// ==========================================================================

template<typename T>
inline auto make_octets_span(T* items, SpanInt count)
{
	constexpr auto item_size = static_cast<SpanInt>(sizeof(T));
	using Octet = std::conditional_t<std::is_const<T>::value, const UInt8, UInt8>;
	const auto size = count * item_size;

	return Span<Octet>{reinterpret_cast<Octet*>(items), size};
}

template<typename T, SpanInt TSize>
inline auto make_octets_span(T (&array)[TSize]) noexcept
{
	constexpr auto item_size = static_cast<SpanInt>(sizeof(T));
	using Octet = std::conditional_t<std::is_const<T>::value, const UInt8, UInt8>;
	const auto size = TSize * item_size;

	return Span<Octet>{reinterpret_cast<Octet*>(array), size};
}

template<typename T>
inline auto make_octets_span(Span<T> span) noexcept
{
	using Octet = std::conditional_t<std::is_const<T>::value, const UInt8, UInt8>;

	return Span<Octet>{reinterpret_cast<Octet*>(span.get_data()), span.get_bytes_size()};
}

// ==========================================================================

template<typename T>
inline Span<const UInt8> make_const_octets_span(T* items, SpanInt count)
{
	constexpr auto item_size = static_cast<SpanInt>(sizeof(T));
	const auto size = count * item_size;

	return Span<const UInt8>{reinterpret_cast<const UInt8*>(items), size};
}

template<typename T, SpanInt TSize>
inline Span<const UInt8> make_const_octets_span(T (&array)[TSize]) noexcept
{
	constexpr auto item_size = static_cast<SpanInt>(sizeof(T));
	const auto size = TSize * item_size;

	return Span<const UInt8>{reinterpret_cast<const UInt8*>(array), size};
}

template<typename T>
inline Span<const UInt8> make_const_octets_span(Span<T> span) noexcept
{
	return Span<const UInt8>{reinterpret_cast<const UInt8*>(span.get_data()), span.get_bytes_size()};
}

} // namespace bstone

#endif // BSTONE_SPAN_INCLUDED
