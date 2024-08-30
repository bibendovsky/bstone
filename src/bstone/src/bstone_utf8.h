/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// UTF-8 primitives.

#ifndef BSTONE_UTF8_INCLUDED
#define BSTONE_UTF8_INCLUDED

#include <cstdint>

#include <type_traits>

#include "bstone_exception.h"
#include "bstone_unicode.h"

namespace bstone {
namespace utf8 {

constexpr auto min_byte_1 = 0x00;
constexpr auto max_byte_1 = 0x7F;

constexpr auto min_byte_2 = 0xC0;
constexpr auto max_byte_2 = 0xDF;

constexpr auto min_byte_3 = 0xE0;
constexpr auto max_byte_3 = 0xEF;

constexpr auto min_byte_4 = 0xF0;
constexpr auto max_byte_4 = 0xF7;

constexpr auto min_byte_n = 0x80;
constexpr auto max_byte_n = 0xBF;

constexpr auto byte_n_mask = 0B00111111;

constexpr auto byte_1_min_range = 0x000000;
constexpr auto byte_1_max_range = 0x00007F;

constexpr auto byte_2_min_range = 0x000080;
constexpr auto byte_2_max_range = 0x0007FF;

constexpr auto byte_3_min_range = 0x000800;
constexpr auto byte_3_max_range = 0x00FFFF;

constexpr auto byte_4_min_range = 0x010000;

// ==========================================================================

template<typename TCodeUnitIter, typename TCodePoint>
inline void decode_code_point(
	TCodeUnitIter code_units_begin,
	TCodeUnitIter code_units_end,
	TCodeUnitIter& code_units_next,
	TCodePoint& code_point)
{
	constexpr auto invalid_sequence_message = "Invalid sequence.";
	constexpr auto overlong_encoding_message = "Overlong encoding.";

	const auto get_code_unit = [code_units_end, &code_units_next]()
	{
		if (code_units_next == code_units_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Truncated sequence.");
		}

		const auto code_unit = static_cast<std::uint8_t>(*code_units_next);
		++code_units_next;
		return code_unit;
	};

	code_units_next = code_units_begin;

	const auto byte_1 = get_code_unit();

	if (byte_1 <= max_byte_1)
	{
		code_point = byte_1;
	}
	else if (byte_1 >= min_byte_2 && byte_1 <= max_byte_2)
	{
		const auto byte_2 = get_code_unit();

		code_point = static_cast<TCodePoint>(
			((byte_1 - min_byte_2) << 6) |
			((byte_2 - min_byte_n) << 0));

		if (code_point < byte_2_min_range || code_point > byte_2_max_range)
		{
			BSTONE_THROW_STATIC_SOURCE(overlong_encoding_message);
		}
	}
	else if (byte_1 >= min_byte_3 && byte_1 <= max_byte_3)
	{
		const auto byte_2 = get_code_unit();

		if (byte_2 < min_byte_n || byte_2 > max_byte_n)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		const auto byte_3 = get_code_unit();

		if (byte_3 < min_byte_n || byte_3 > max_byte_n)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		code_point = static_cast<TCodePoint>(
			((byte_1 - min_byte_3) << 12) |
			((byte_2 - min_byte_n) << 6) |
			((byte_3 - min_byte_n) << 0));

		if (code_point < byte_3_min_range || code_point > byte_3_max_range)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		if (unicode::is_code_point_surrogate(code_point))
		{
			BSTONE_THROW_STATIC_SOURCE("Surrogate code point.");
		}
	}
	else if (byte_1 >= min_byte_4 && byte_1 <= max_byte_4)
	{
		const auto byte_2 = get_code_unit();

		if (byte_2 < min_byte_n || byte_2 > max_byte_n)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		const auto byte_3 = get_code_unit();

		if (byte_3 < min_byte_n || byte_3 > max_byte_n)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		const auto byte_4 = get_code_unit();

		if (byte_4 < min_byte_n || byte_4 > max_byte_n)
		{
			BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
		}

		code_point = static_cast<TCodePoint>(
			((byte_1 - min_byte_4) << 18) |
			((byte_2 - min_byte_n) << 12) |
			((byte_3 - min_byte_n) << 6) |
			((byte_4 - min_byte_n) << 0));

		if (code_point < byte_4_min_range)
		{
			BSTONE_THROW_STATIC_SOURCE(overlong_encoding_message);
		}
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE(invalid_sequence_message);
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

		*code_units_next = static_cast<CodeUnit>(static_cast<std::uint8_t>(code_unit));
		++code_units_next;
	};

	code_units_next = code_units_begin;

	if (code_point >= byte_1_min_range && code_point <= byte_1_max_range)
	{
		set_code_unit(code_point);
	}
	else if (code_point >= byte_2_min_range && code_point <= byte_2_max_range)
	{
		set_code_unit(min_byte_2 | (code_point >> 6));
		set_code_unit(min_byte_n | (code_point & byte_n_mask));
	}
	else if (code_point >= byte_3_min_range && code_point <= byte_3_max_range)
	{
		if (unicode::is_code_point_surrogate(code_point))
		{
			BSTONE_THROW_STATIC_SOURCE("Surrogate code point.");
		}

		set_code_unit(min_byte_3 | (code_point >> 12));
		set_code_unit(min_byte_n | ((code_point >> 6) & byte_n_mask));
		set_code_unit(min_byte_n | (code_point & byte_n_mask));
	}
	else if (code_point >= byte_4_min_range && code_point <= static_cast<TCodePoint>(unicode::max_code_point))
	{
		set_code_unit(min_byte_4 | (code_point >> 18));
		set_code_unit(min_byte_n | ((code_point >> 12) & byte_n_mask));
		set_code_unit(min_byte_n | ((code_point >> 6) & byte_n_mask));
		set_code_unit(min_byte_n | (code_point & byte_n_mask));
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Code point out of range.");
	}
}

} // namespace utf8
} // namespace bstone

#endif // BSTONE_UTF8_INCLUDED
