/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_CHAR_CONV_INCLUDED)
#define BSTONE_CHAR_CONV_INCLUDED

#include <cassert>

#include <limits>
#include <type_traits>

#include "bstone_ascii.h"
#include "bstone_exception.h"
#include "bstone_int.h"
#include "bstone_utility.h"

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

// ==========================================================================

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

// ==========================================================================

template<typename TChar, typename TByte>
inline constexpr TByte* hex_chars_to_bytes(
	const TChar* chars_begin,
	const TChar* chars_end,
	TByte* bytes_begin,
	TByte* bytes_end)
{
	static_assert(sizeof(TByte) == 1, "Expected one-byte-size type.");

	const auto char_count = chars_end - chars_begin;

	if (char_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Char count out of range.");
	}

	if ((char_count % 2) != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Odd char count.");
	}

	const auto byte_count = bytes_end - bytes_begin;

	if (byte_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte count out of range.");
	}

	const auto char_half_count = char_count / 2;

	if (byte_count < char_half_count)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte buffer too small.");
	}

	auto i_char = IntP{};

	for (auto i_byte = IntP{}; i_byte < char_half_count; ++i_byte)
	{
		const auto high_nibble = bstone::hex_char_to_nibble<TByte>(chars_begin[i_char++]);
		const auto low_nibble = bstone::hex_char_to_nibble<TByte>(chars_begin[i_char++]);
		const auto byte = static_cast<TByte>((high_nibble << 4) | low_nibble);
		bytes_begin[i_byte] = byte;
	}

	return bytes_begin + char_half_count;
}

// ==========================================================================

template<typename TByte, typename TChar>
inline constexpr TChar* bytes_to_hex_chars(
	const TByte* bytes_begin,
	const TByte* bytes_end,
	TChar* chars_begin,
	TChar* chars_end)
{
	static_assert(sizeof(TByte) == 1, "Expected one-byte-size type.");

	const auto byte_count = bytes_end - bytes_begin;

	if (byte_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Byte count out of range.");
	}

	const auto char_count = chars_end - chars_begin;

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

	auto i_char = IntP{};

	for (auto i_byte = IntP{}; i_byte < byte_count; ++i_byte)
	{
		const auto byte = static_cast<Unsigned>(static_cast<std::make_unsigned_t<TByte>>(bytes_begin[i_byte]));
		const auto char_1 = bstone::nibble_to_hex_char<TChar>((byte >> 4) & 0xF);
		const auto char_2 = bstone::nibble_to_hex_char<TChar>(byte & 0xF);
		chars_begin[i_char++] = char_1;
		chars_begin[i_char++] = char_2;
	}

	return chars_begin + i_char;
}

// ==========================================================================

constexpr auto char_conv_min_base = 2;
constexpr auto char_conv_max_base = 36;

// --------------------------------------------------------------------------

namespace detail {

struct ToCharsIntegralTag {};

template<typename T>
struct ToCharsIntegralToUnsigned
{
	using Unsigned = std::make_unsigned_t<T>;

	template<typename U = T, std::enable_if_t<std::is_signed<U>::value, int> = 0>
	constexpr Unsigned operator()(U value) const noexcept
	{
		return static_cast<Unsigned>((value >= 0 || value == (std::numeric_limits<U>::min)()) ? value : -value);
	}

	template<typename U = T, std::enable_if_t<std::is_unsigned<U>::value, int> = 0>
	constexpr Unsigned operator()(U value) const noexcept
	{
		return value;
	}
};

} // namespace detail

template<typename TValue, typename TChar>
inline constexpr TChar* to_chars(TValue value, TChar* chars_begin, TChar* chars_end, int base = 10)
{
	static_assert(std::is_integral<TValue>::value, "Expected an integral type.");

	if (base < char_conv_min_base || base > char_conv_max_base)
	{
		BSTONE_THROW_STATIC_SOURCE("Base out of range.");
	}

	constexpr auto buffer_too_small_string = "Buffer too small.";

	const auto is_minus_sign = value < 0;

	auto u_value = detail::ToCharsIntegralToUnsigned<TValue>{}(value);

	const auto max_char_count = chars_end - chars_begin;

	if (max_char_count < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Buffer size out of range.");
	}

	auto i_char = IntP{};

	while (true)
	{
		if (i_char == max_char_count)
		{
			BSTONE_THROW_STATIC_SOURCE(buffer_too_small_string);
		}

		const auto next_value = u_value / base;
		const auto digit = u_value - (next_value * base);
		assert(digit < static_cast<decltype(digit)>(base));
		u_value = static_cast<decltype(u_value)>(next_value);
		const auto digit_char = static_cast<TChar>(digit < 10 ? ('0' + digit) : ('a' + digit - 10));
		chars_begin[i_char++] = digit_char;

		if (next_value == 0)
		{
			break;
		}
	}

	auto sign = TChar{};

	if (is_minus_sign)
	{
		sign = '-';
	}

	if (sign != TChar{})
	{
		if (i_char == max_char_count)
		{
			BSTONE_THROW_STATIC_SOURCE(buffer_too_small_string);
		}

		chars_begin[i_char++] = sign;
	}

	const auto half_char_count = i_char / 2;

	for (auto i = IntP{}; i < half_char_count; ++i)
	{
		bstone::swop(chars_begin[i], chars_begin[i_char - 1 - i]);
	}

	return chars_begin + i_char;
}

// ==========================================================================

namespace detail {

template<typename T, typename TUnsigned>
struct FromCharsIntegralFromUnsigned
{
	template<typename U = T, std::enable_if_t<std::is_signed<U>::value, int> = 0>
	constexpr U operator()(TUnsigned u_value, bool is_negative) const noexcept
	{
		return is_negative ? (-static_cast<U>(u_value)) : static_cast<U>(u_value);
	}

	template<typename U = T, std::enable_if_t<std::is_unsigned<U>::value, int> = 0>
	constexpr U operator()(TUnsigned u_value, bool) const noexcept
	{
		return u_value;
	}
};

} // namespace detail

template<typename TChar, typename TValue>
inline constexpr const TChar* from_chars(
	const TChar* chars_begin,
	const TChar* chars_end,
	TValue& value,
	int base = 10)
{
	const auto max_char_count = chars_end - chars_begin;

	if (max_char_count <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Char count out of range.");
	}

	if (base != 0 && (base < char_conv_min_base || base > char_conv_max_base))
	{
		BSTONE_THROW_STATIC_SOURCE("Base out of range.");
	}

	auto i_char = IntP{};
	auto has_minus_sign = false;

	if (chars_begin[i_char] == '-')
	{
		if (!std::is_signed<TValue>::value)
		{
			BSTONE_THROW_STATIC_SOURCE("Negative value for unsigned type.");
		}

		has_minus_sign = true;
		++i_char;
	}
	else if (chars_begin[i_char] == '+')
	{
		++i_char;
	}

	if (base == 0)
	{
		auto detected_base = 0;

		if (i_char + 2 <= max_char_count &&
			chars_begin[i_char + 0] == '0' &&
			(chars_begin[i_char + 1] == 'x' || chars_begin[i_char + 1] == 'X'))
		{
			detected_base = 16;
			i_char += 2;
		}
		else if (i_char + 1 <= max_char_count)
		{
			if (chars_begin[i_char] == '0')
			{
				detected_base = 8;
				++i_char;
			}
			else if (ascii::is_decimal(chars_begin[i_char]))
			{
				detected_base = 10;
			}
		}

		if (detected_base == 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Unable to detect a base.");
		}

		base = detected_base;
	}

	if (i_char == max_char_count)
	{
		BSTONE_THROW_STATIC_SOURCE("No digits.");
	}

	const auto max_value = (
		std::is_signed<TValue>::value ?
			(has_minus_sign ? (std::numeric_limits<TValue>::min)() : (std::numeric_limits<TValue>::max)()) :
				(std::numeric_limits<TValue>::max)());

	using Unsigned = std::make_unsigned_t<TValue>;
	const auto max_u_value = static_cast<Unsigned>(max_value);
	const auto max_prev_u_value = max_u_value / base;
	const auto max_last_digit = static_cast<TChar>(max_u_value - (max_prev_u_value * base));

	auto u_value = Unsigned{};

	while (i_char != max_char_count)
	{
		const auto digit_char = chars_begin[i_char++];
		auto digit = TChar{};

		if (ascii::is_decimal(digit_char))
		{
			digit = digit_char - '0';
		}
		else if (ascii::is_lower(digit_char))
		{
			digit = 10 + digit_char - 'a';
		}
		else if (ascii::is_upper(digit_char))
		{
			digit = 10 + digit_char - 'A';
		}
		else
		{
			BSTONE_THROW_STATIC_SOURCE("Invalid digit character.");
		}

		if (digit >= base)
		{
			BSTONE_THROW_STATIC_SOURCE("Digit character out of range.");
		}

		if (u_value > max_prev_u_value || (u_value == max_prev_u_value && digit > max_last_digit))
		{
			BSTONE_THROW_STATIC_SOURCE("Number overflow.");
		}

		u_value *= static_cast<Unsigned>(base);
		u_value += static_cast<Unsigned>(digit);
	}

	value = detail::FromCharsIntegralFromUnsigned<TValue, Unsigned>{}(u_value, has_minus_sign);
	return chars_begin + i_char;
}

} // namespace bstone

#endif // BSTONE_CHAR_CONV_INCLUDED
