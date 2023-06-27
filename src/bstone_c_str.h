/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_C_STR_INCLUDED
#define BSTONE_C_STR_INCLUDED

#include <cassert>
#include <type_traits>
#include <utility>
#include "bstone_span.h"

namespace bstone {
namespace c_str {

template<typename TChar>
inline constexpr IntP get_size(const TChar* chars)
{
	auto i_chars = chars;

	while (*i_chars != TChar{})
	{
		++i_chars;
	}

	return i_chars - chars;
}

// --------------------------------------------------------------------------

template<typename TChar>
inline constexpr IntP get_size_with_null(const TChar* chars)
{
	return get_size(chars) + 1;
}

// ==========================================================================

template<typename TChar>
constexpr int compare(
	const TChar* lhs_chars,
	IntP lhs_size,
	const TChar* rhs_chars,
	IntP rhs_size) noexcept
{
	const auto size = lhs_size < rhs_size ? lhs_size : rhs_size;

	for (auto i = decltype(size){}; i < size; ++i)
	{
		const auto& lhs_char = lhs_chars[i];
		const auto& rhs_char = rhs_chars[i];

		if (lhs_char < rhs_char)
		{
			return -1;
		}
		else if (lhs_char > rhs_char)
		{
			return 1;
		}
	}

	if (lhs_size < rhs_size)
	{
		return -1;
	}
	else if (lhs_size > rhs_size)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

// ==========================================================================

namespace detail {

struct MakeSpanArrayTag {};
struct MakeSpanPointerTag {};

template<typename TChar, IntP TSize>
inline constexpr auto make_span(TChar (&string)[TSize], MakeSpanArrayTag)
{
	assert(string[TSize - 1] == TChar{});
	return bstone::make_span(string, TSize - 1);
}

template<typename TChar>
inline constexpr auto make_span(TChar* string, MakeSpanPointerTag)
{
	assert(string != nullptr);
	return bstone::make_span(string, get_size(string));
}

} // namespace detail

template<typename TObject>
inline constexpr auto make_span(TObject&& object)
{
	using Decayed = std::decay_t<TObject>;

	using Tag = std::conditional_t<
		std::is_array<Decayed>::value,
		detail::MakeSpanArrayTag,
		std::conditional_t<
			std::is_pointer<Decayed>::value,
			detail::MakeSpanPointerTag,
			void>>;

	static_assert(!std::is_same<Tag, void>::value, "Unsupported type.");

	return detail::make_span(std::forward<TObject>(object), Tag{});
}

// --------------------------------------------------------------------------

namespace detail {

struct MakeSpanWithNullArrayTag {};
struct MakeSpanWithNullPointerTag {};

template<typename TChar, IntP TSize>
inline constexpr auto make_span_with_null(TChar (&string)[TSize], MakeSpanWithNullArrayTag)
{
	assert(string[TSize - 1] == TChar{});
	return bstone::make_span(string, TSize);
}

template<typename TChar>
inline constexpr Span<TChar> make_span_with_null(TChar* string, MakeSpanWithNullPointerTag)
{
	assert(string != nullptr);
	return bstone::make_span(string, get_size_with_null(string));
}

} // namespace detail

template<typename TObject>
inline constexpr auto make_span_with_null(TObject&& object)
{
	using Decayed = std::decay_t<TObject>;

	using Tag = std::conditional_t<
		std::is_array<Decayed>::value,
		detail::MakeSpanWithNullArrayTag,
		std::conditional_t<
			std::is_pointer<Decayed>::value,
			detail::MakeSpanWithNullPointerTag,
			void>>;

	static_assert(!std::is_same<Tag, void>::value, "Unsupported type.");

	return detail::make_span_with_null(std::forward<TObject>(object), Tag{});
}

} // namespace c_str
} // namespace bstone

#endif // !BSTONE_C_STR_INCLUDED
