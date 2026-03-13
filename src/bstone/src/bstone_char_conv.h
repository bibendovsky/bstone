/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Character sequence to integer utility

#ifndef BSTONE_CHAR_CONV_INCLUDED
#define BSTONE_CHAR_CONV_INCLUDED

#include "bstone_ascii.h"
#include "bstone_exception.h"
#include <cstdint>
#include <type_traits>

namespace bstone {

template<typename TChar, typename TNibble>
inline constexpr TChar nibble_to_hex_char(TNibble nibble)
{
	if (nibble >= 0 && nibble <= 9)
	{
		return static_cast<TChar>(nibble + '0');
	}
	else if (nibble >= 0xA && nibble <= 0xF)
	{
		return static_cast<TChar>(nibble - 0xA + 'a');
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Nibble out of range.");
	}
}

// =====================================

template<typename TNibble, typename TChar>
inline constexpr TNibble hex_char_to_nibble(TChar hex_char)
{
	if (ascii::is_decimal(hex_char))
	{
		return static_cast<TNibble>(hex_char - '0');
	}
	else if (hex_char >= 'a' && hex_char <= 'f')
	{
		return static_cast<TNibble>(0xA + hex_char - 'a');
	}
	else if (hex_char >= 'A' && hex_char <= 'F')
	{
		return static_cast<TNibble>(0xA + hex_char - 'A');
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid hex character.");
	}
}

// =====================================

template<typename TChar, typename TByte>
inline constexpr TByte* hex_chars_to_bytes(
	const TChar* chars_begin,
	const TChar* chars_end,
	TByte* bytes_begin,
	TByte* bytes_end)
{
	static_assert(sizeof(TByte) == 1, "Expected one-byte-size type.");
	const std::intptr_t char_count = chars_end - chars_begin;
	if (char_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Char count out of range.");
	}
	if ((char_count % 2) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Odd char count.");
	}
	const std::intptr_t byte_count = bytes_end - bytes_begin;
	if (byte_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte count out of range.");
	}
	const std::intptr_t char_half_count = char_count / 2;
	if (byte_count < char_half_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte buffer too small.");
	}
	std::intptr_t i_char = 0;
	for (std::intptr_t i_byte = 0; i_byte < char_half_count; ++i_byte)
	{
		const auto high_nibble = bstone::hex_char_to_nibble<TByte>(chars_begin[i_char++]);
		const auto low_nibble = bstone::hex_char_to_nibble<TByte>(chars_begin[i_char++]);
		const TByte byte = static_cast<TByte>((high_nibble << 4) | low_nibble);
		bytes_begin[i_byte] = byte;
	}
	return bytes_begin + char_half_count;
}

// =====================================

template<typename TByte, typename TChar>
inline constexpr TChar* bytes_to_hex_chars(
	const TByte* bytes_begin,
	const TByte* bytes_end,
	TChar* chars_begin,
	TChar* chars_end)
{
	static_assert(sizeof(TByte) == 1, "Expected one-byte-size type.");
	const std::intptr_t byte_count = bytes_end - bytes_begin;
	if (byte_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte count out of range.");
	}
	const std::intptr_t char_count = chars_end - chars_begin;
	if (char_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Char count out of range.");
	}
	if (char_count < byte_count * 2)
	{
		BSTONE_THROW_STATIC_SOURCE("Char buffer too small.");
	}
	using Unsigned = std::conditional_t<
		sizeof(TByte) >= sizeof(unsigned int),
		std::make_unsigned_t<TByte>,
		unsigned int>;
	std::intptr_t i_char = 0;
	for (std::intptr_t i_byte = 0; i_byte < byte_count; ++i_byte)
	{
		const Unsigned byte = static_cast<Unsigned>(
			static_cast<std::make_unsigned_t<TByte>>(bytes_begin[i_byte]));
		const TChar char_1 = bstone::nibble_to_hex_char<TChar>((byte >> 4) & 0xF);
		const TChar char_2 = bstone::nibble_to_hex_char<TChar>(byte & 0xF);
		chars_begin[i_char++] = char_1;
		chars_begin[i_char++] = char_2;
	}
	return chars_begin + i_char;
}

} // namespace bstone

#endif // BSTONE_CHAR_CONV_INCLUDED
