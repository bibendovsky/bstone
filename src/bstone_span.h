/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A contiguous sequence of objects.

#ifndef BSTONE_SPAN_INCLUDED
#define BSTONE_SPAN_INCLUDED

#include <cstdint>

#include <type_traits>

#include "bstone_assert.h"
#include "bstone_utility.h"

namespace bstone {

template<typename T>
class Span
{
public:
	using Item = T;

public:
	Span() = default;

	constexpr Span(Item* items, std::intptr_t size)
		:
		items_{items},
		size_{size}
	{
		BSTONE_ASSERT((items == nullptr && size == 0) || (items != nullptr && size >= 0));
	}

	template<std::intptr_t TSize>
	constexpr explicit Span(Item (&items)[TSize]) noexcept
		:
		items_{items},
		size_{TSize}
	{}

	constexpr Item* get_data() const noexcept
	{
		return items_;
	}

	constexpr std::intptr_t get_size() const noexcept
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

	constexpr std::intptr_t get_bytes_size() const noexcept
	{
		return get_size() * static_cast<std::intptr_t>(sizeof(Item));
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
		BSTONE_ASSERT(has_data());
		BSTONE_ASSERT(!is_empty());

		return *(begin());
	}

	constexpr Item& get_front()
	{
		return ::bstone::as_mutable(::bstone::as_const(*this).get_front());
	}

	constexpr const Item& get_back() const
	{
		BSTONE_ASSERT(has_data());
		BSTONE_ASSERT(!is_empty());

		return *(end() - 1);
	}

	constexpr Item& get_back()
	{
		return ::bstone::as_mutable(::bstone::as_const(*this).get_back());
	}

	constexpr Span get_subspan(std::intptr_t offset, std::intptr_t size) const
	{
		BSTONE_ASSERT(offset >= 0);
		BSTONE_ASSERT(size >= 0);
		BSTONE_ASSERT((offset + size) <= get_size());

		return Span{get_data() + offset, size};
	}

	constexpr Span get_subspan(std::intptr_t offset) const
	{
		return get_subspan(offset, get_size() - offset);
	}

	constexpr Item& operator[](std::intptr_t index) const
	{
		BSTONE_ASSERT(index >= 0 && index < get_size());
		BSTONE_ASSERT(has_data());

		return get_data()[index];
	}

	constexpr void swap(Span& rhs) noexcept
	{
		::bstone::swop(items_, rhs.items_);
		::bstone::swop(size_, rhs.size_);
	}

private:
	Item* items_{};
	std::intptr_t size_{};
};

// ==========================================================================

template<typename T>
inline constexpr auto make_span(T* items, std::intptr_t size)
{
	return Span<T>{items, size};
}

template<typename T, std::intptr_t TSize>
inline constexpr auto make_span(T (&array)[TSize]) noexcept
{
	return Span<T>{array};
}

// ==========================================================================

template<typename T>
inline constexpr auto make_const_span(T* items, std::intptr_t size)
{
	return Span<const T>{items, size};
}

template<typename T, std::intptr_t TSize>
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
inline auto make_octets_span(T* items, std::intptr_t count)
{
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	using Octet = std::conditional_t<std::is_const<T>::value, const std::uint8_t, std::uint8_t>;
	const auto size = count * item_size;

	return Span<Octet>{reinterpret_cast<Octet*>(items), size};
}

template<typename T, std::intptr_t TSize>
inline auto make_octets_span(T (&array)[TSize]) noexcept
{
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	using Octet = std::conditional_t<std::is_const<T>::value, const std::uint8_t, std::uint8_t>;
	const auto size = TSize * item_size;

	return Span<Octet>{reinterpret_cast<Octet*>(array), size};
}

template<typename T>
inline auto make_octets_span(Span<T> span) noexcept
{
	using Octet = std::conditional_t<std::is_const<T>::value, const std::uint8_t, std::uint8_t>;

	return Span<Octet>{reinterpret_cast<Octet*>(span.get_data()), span.get_bytes_size()};
}

// ==========================================================================

template<typename T>
inline Span<const std::uint8_t> make_const_octets_span(T* items, std::intptr_t count)
{
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	const auto size = count * item_size;

	return Span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(items), size};
}

template<typename T, std::intptr_t TSize>
inline Span<const std::uint8_t> make_const_octets_span(T (&array)[TSize]) noexcept
{
	constexpr auto item_size = static_cast<std::intptr_t>(sizeof(T));
	const auto size = TSize * item_size;

	return Span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(array), size};
}

template<typename T>
inline Span<const std::uint8_t> make_const_octets_span(Span<T> span) noexcept
{
	return Span<const std::uint8_t>{
		reinterpret_cast<const std::uint8_t*>(span.get_data()),
		span.get_bytes_size()};
}

} // namespace bstone

#endif // BSTONE_SPAN_INCLUDED
