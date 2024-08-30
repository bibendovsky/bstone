/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CHAR_CONV_INCLUDED
#define BSTONE_CHAR_CONV_INCLUDED

#include <cstdint>

#include <limits>
#include <type_traits>

#include "bstone_ascii.h"
#include "bstone_exception.h"
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

	auto i_char = std::intptr_t{};

	for (auto i_byte = std::intptr_t{}; i_byte < char_half_count; ++i_byte)
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

	auto i_char = std::intptr_t{};

	for (auto i_byte = std::intptr_t{}; i_byte < byte_count; ++i_byte)
	{
		const auto byte = static_cast<Unsigned>(
			static_cast<std::make_unsigned_t<TByte>>(bytes_begin[i_byte]));
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

template<typename TValue, bool TIsSigned = std::is_signed<TValue>::value>
struct ToCharsAbs
{
	TValue operator()(TValue value) const
	{
		return value < 0 ? -value : value;
	}
};

template<typename TValue>
struct ToCharsAbs<TValue, false>
{
	TValue operator()(TValue value) const
	{
		return value;
	}
};

} // namespace detail

template<typename TValue, typename TCharIter>
inline constexpr TCharIter to_chars(
	TValue value,
	TCharIter chars_begin,
	TCharIter chars_end,
	int base = 10)
{
	static_assert(std::is_integral<TValue>::value && sizeof(TValue) <= 8, "Unsupported type.");

	if (base < char_conv_min_base || base > char_conv_max_base)
	{
		BSTONE_THROW_STATIC_SOURCE("Base out of range.");
	}

	using Char = std::remove_reference_t<decltype(*chars_begin)>;
	constexpr auto abs = detail::ToCharsAbs<TValue>{};

	constexpr auto max_chars = std::intptr_t{20}; // 64-bit
	Char chars[max_chars];

	const auto is_negative = value < 0;
	const auto v_base = static_cast<TValue>(base);
	auto chars_count = std::intptr_t{};

	while (true)
	{
		const auto next_value = static_cast<TValue>(value / v_base);
		const auto digit = abs(value % v_base);
		value = next_value;
		const auto digit_char = static_cast<Char>(digit < 10 ? ('0' + digit) : ('a' + digit - 10));
		chars[chars_count++] = digit_char;

		if (next_value == 0)
		{
			break;
		}
	}

	if (is_negative)
	{
		chars[chars_count++] = static_cast<Char>('-');
	}

	for (auto i = 0; i < chars_count; ++i)
	{
		if (chars_begin == chars_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
		}

		*chars_begin = chars[chars_count - 1 - i];
		++chars_begin;
	}

	return chars_begin;
}

// ==========================================================================

namespace detail {

struct FromCharsErrorMessages
{
	static const char* number_overflow;
	static const char* invalid_character;
	static const char* digit_out_of_range;
	static const char* negative_unsigned;
	static const char* base_out_of_range;
	static const char* unexpected_end_of_chars;
	static const char* unable_to_detect_a_base;
};

struct FromCharsSignedTag {};
struct FromCharsUnsignedTag {};

template<typename TValue, typename TChar>
inline constexpr TValue from_chars_char_to_digit(TChar ch, int base)
{
	auto digit = 0;

	if (ch >= '0' && ch <= '9')
	{
		digit = ch - '0';
	}
	else if (ch >= 'a' && ch <= 'z')
	{
		digit = 0xA + ch - 'a';
	}
	else if (ch >= 'A' && ch <= 'Z')
	{
		digit = 0xA + ch - 'A';
	}
	else
	{
		BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::invalid_character);
	}

	if (digit < 0 || digit >= base)
	{
		BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::digit_out_of_range);
	}

	return static_cast<TValue>(digit);
}

template<typename TCharIter, typename TValue>
inline constexpr TCharIter from_chars_signed_negative(
	TCharIter chars_begin,
	TCharIter chars_end,
	TValue& value,
	int base)
{
	constexpr auto min_value = (std::numeric_limits<TValue>::min)();

	const auto min_mul_value = min_value / static_cast<TValue>(base);

	value = TValue{};

	while (chars_begin != chars_end)
	{
		const auto ch = *chars_begin;
		++chars_begin;

		if (ch != '0')
		{
			value = -detail::from_chars_char_to_digit<TValue>(ch, base);
			break;
		}
	}

	for (; chars_begin != chars_end; ++chars_begin)
	{
		const auto digit = detail::from_chars_char_to_digit<TValue>(*chars_begin, base);

		if (value < min_mul_value)
		{
			BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::number_overflow);
		}

		value *= static_cast<TValue>(base);

		if (value < min_value + digit)
		{
			BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::number_overflow);
		}

		value -= digit;
	}

	return chars_begin;
}

template<typename TCharIter, typename TValue>
inline constexpr TCharIter from_chars_signed_positive_or_unsigned(
	TCharIter chars_begin,
	TCharIter chars_end,
	TValue& value,
	int base)
{
	constexpr auto max_value = (std::numeric_limits<TValue>::max)();

	const auto max_mul_value = max_value / static_cast<TValue>(base);

	value = TValue{};

	for (; chars_begin != chars_end; ++chars_begin)
	{
		const auto digit = detail::from_chars_char_to_digit<TValue>(*chars_begin, base);

		if (value > max_mul_value)
		{
			BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::number_overflow);
		}

		value *= static_cast<TValue>(base);

		if (value > max_value - digit)
		{
			BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::number_overflow);
		}

		value += digit;
	}

	return chars_begin;
}

template<typename TCharIter, typename TValue>
inline constexpr TCharIter from_chars(
	TCharIter chars_begin,
	TCharIter chars_end,
	TValue& value,
	int base,
	bool is_negative,
	FromCharsSignedTag)
{
	if (is_negative)
	{
		return detail::from_chars_signed_negative(chars_begin, chars_end, value, base);
	}
	else
	{
		return detail::from_chars_signed_positive_or_unsigned(chars_begin, chars_end, value, base);
	}
}

template<typename TCharIter, typename TValue>
inline constexpr TCharIter from_chars(
	TCharIter chars_begin,
	TCharIter chars_end,
	TValue& value,
	int base,
	bool is_negative,
	FromCharsUnsignedTag)
{
	if (is_negative)
	{
		BSTONE_THROW_STATIC_SOURCE(FromCharsErrorMessages::negative_unsigned);
	}

	return detail::from_chars_signed_positive_or_unsigned(chars_begin, chars_end, value, base);
}

} // namespace detail

template<typename TCharIter, typename TValue>
inline constexpr TCharIter from_chars(
	TCharIter chars_begin,
	TCharIter chars_end,
	TValue& value,
	int base = 10)
{
	if (base != 0 && (base < char_conv_min_base || base > char_conv_max_base))
	{
		BSTONE_THROW_STATIC_SOURCE(detail::FromCharsErrorMessages::base_out_of_range);
	}

	auto is_negative = false;
	auto chars_iter = chars_begin;

	const auto ensure_chars = [&chars_iter, chars_end]()
	{
		if (chars_iter == chars_end)
		{
			BSTONE_THROW_STATIC_SOURCE(detail::FromCharsErrorMessages::unexpected_end_of_chars);
		}
	};

	ensure_chars();

	if (*chars_iter == '-')
	{
		is_negative = true;
		++chars_iter;
	}
	else if (*chars_iter == '+')
	{
		is_negative = true;
		++chars_iter;
	}

	if (base == 0)
	{
		auto detected_base = 0;

		if (chars_iter != chars_end)
		{
			const auto char_0 = *chars_iter;

			if (char_0 == '0')
			{
				++chars_iter;

				if (chars_iter == chars_end)
				{
					value = 0;
					return chars_iter;
				}

				const auto char_1 = *chars_iter;

				if (char_1 == 'x' || char_1 == 'X')
				{
					++chars_iter;
					detected_base = 16;
				}
				else
				{
					detected_base = 8;
				}
			}
			else
			{
				if (ascii::is_decimal(char_0))
				{
					detected_base = 10;
				}
			}
		}

		if (detected_base == 0)
		{
			BSTONE_THROW_STATIC_SOURCE(detail::FromCharsErrorMessages::unable_to_detect_a_base);
		}

		base = detected_base;
	}

	ensure_chars();

	using Tag = std::conditional_t<
		std::is_signed<TValue>::value,
		detail::FromCharsSignedTag,
		detail::FromCharsUnsignedTag>;

	return detail::from_chars(chars_iter, chars_end, value, base, is_negative, Tag{});
}

} // namespace bstone

#endif // BSTONE_CHAR_CONV_INCLUDED
