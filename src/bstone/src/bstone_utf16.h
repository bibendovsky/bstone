/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// UTF-16 primitives.

#ifndef BSTONE_UTF16_INCLUDED
#define BSTONE_UTF16_INCLUDED

#include <cstdint>

#include <type_traits>

#include "bstone_exception.h"
#include "bstone_unicode.h"

namespace bstone {
namespace utf16 {

constexpr auto min_code_unit_1 = 0x000000;
constexpr auto max_code_unit_1 = 0x00FFFF;

constexpr auto min_code_unit_2 = 0x010000;

// ==========================================================================

template<typename TCodeUnitIter, typename TCodePoint>
inline void decode_code_point(
	TCodeUnitIter code_units_begin,
	TCodeUnitIter code_units_end,
	TCodeUnitIter& code_units_next,
	TCodePoint& code_point)
{
	code_units_next = code_units_begin;

	const auto get_code_unit = [code_units_end, &code_units_next]()
	{
		if (code_units_next == code_units_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Truncated sequence.");
		}

		const auto code_unit = static_cast<unicode::CodePoint>(static_cast<std::uint16_t>(*code_units_next));
		++code_units_next;
		return code_unit;
	};

	const auto code_unit_1 = get_code_unit();

	if (code_unit_1 >= unicode::min_high_surrogate_code_point &&
		code_unit_1 <= unicode::max_high_surrogate_code_point)
	{
		const auto code_unit_2 = get_code_unit();

		if (code_unit_2 < unicode::min_low_surrogate_code_point ||
			code_unit_2 > unicode::max_low_surrogate_code_point)
		{
			BSTONE_THROW_STATIC_SOURCE("Expected low surrogate code unit.");
		}

		code_point = static_cast<TCodePoint>(
			min_code_unit_2 + (((code_unit_1 & 0x0003FF) << 10) |
				(code_unit_2 & 0x0003FF)));
	}
	else
	{
		if (code_unit_1 >= unicode::min_low_surrogate_code_point &&
			code_unit_1 <= unicode::max_low_surrogate_code_point)
		{
			BSTONE_THROW_STATIC_SOURCE("Unexpected low surrogate code unit.");
		}

		code_point = static_cast<TCodePoint>(code_unit_1);
	}
}

// ==========================================================================

template<typename TCodeUnitIter, typename TCodePoint>
inline void encode_code_point(
	TCodePoint code_point,
	TCodeUnitIter code_units_begin,
	TCodeUnitIter code_units_end,
	TCodeUnitIter& code_units_next)
{
	using CodeUnit = std::decay_t<decltype(*code_units_begin)>;

	const auto set_code_unit = [code_units_end, &code_units_next](TCodePoint code_unit)
	{
		if (code_units_next == code_units_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Destination buffer too small.");
		}

		*code_units_next = static_cast<CodeUnit>(static_cast<std::uint16_t>(code_unit));
		++code_units_next;
	};

	code_units_next = code_units_begin;

	if (code_point <= max_code_unit_1)
	{
		set_code_unit(code_point);
	}
	else if (code_point >= min_code_unit_2 && code_point <= unicode::max_code_point)
	{
		code_point -= min_code_unit_2;

		set_code_unit(0xD800 | (code_point >> 10));
		set_code_unit(0xDC00 | (code_point & 0B11'1111'1111U));
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Code point out of range.");
	}
}

} // namespace utf16
} // namespace bstone

#endif // BSTONE_UTF16_INCLUDED
