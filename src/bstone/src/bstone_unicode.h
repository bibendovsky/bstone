/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Unicode primitives.

#ifndef BSTONE_UNICODE_INCLUDED
#define BSTONE_UNICODE_INCLUDED

namespace bstone {
namespace unicode {

using CodePoint = int;

// ==========================================================================

constexpr auto replacement_code_point = CodePoint{0xFFFD};

constexpr auto min_code_point = CodePoint{};
constexpr auto max_code_point = CodePoint{0x10FFFF};

constexpr auto min_high_surrogate_code_point = CodePoint{0xD800};
constexpr auto max_high_surrogate_code_point = CodePoint{0xDBFF};

constexpr auto min_low_surrogate_code_point = CodePoint{0xDC00};
constexpr auto max_low_surrogate_code_point = CodePoint{0xDFFF};

// ==========================================================================

template<typename TCodePoint>
inline constexpr bool is_code_point_surrogate(TCodePoint code_point)
{
	return
		code_point >= static_cast<TCodePoint>(min_high_surrogate_code_point) &&
		code_point <= static_cast<TCodePoint>(max_low_surrogate_code_point);
}

// ==========================================================================

template<typename TCodePoint>
inline constexpr bool is_code_point_valid(TCodePoint code_point)
{
	return
		!is_code_point_surrogate(code_point) &&
		code_point >= static_cast<TCodePoint>(min_code_point) &&
		code_point <= static_cast<TCodePoint>(max_code_point);
}

} // namespace unicode
} // namespace bstone

#endif // BSTONE_UNICODE_INCLUDED
