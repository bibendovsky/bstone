/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
inline constexpr Int get_size(const TChar* string)
{
	assert(string != nullptr);

	const auto string_begin = string;

	while (*string != TChar{})
	{
		++string;
	}

	return string - string_begin;
}

// --------------------------------------------------------------------------

template<typename TChar>
inline constexpr Int get_size_with_null(const TChar* string)
{
	return get_size(string) + 1;
}

// ==========================================================================

namespace detail {

struct MakeSpanArrayTag {};
struct MakeSpanPointerTag {};

template<typename TChar, Int TSize>
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

template<typename TChar, Int TSize>
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
