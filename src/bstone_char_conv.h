/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CHAR_CONV_INCLUDED
#define BSTONE_CHAR_CONV_INCLUDED

#include <cassert>
#include <limits>
#include <type_traits>
#include <bstone_ascii.h>
#include <bstone_enum_flags.h>
#include <bstone_int.h>
#include <bstone_span.h>

namespace bstone {

namespace detail {

[[noreturn]] void fail_nibble_to_hex_char(const char* message);

} // namespace detail

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
		detail::fail_nibble_to_hex_char("Nibble out of range.");
	}
}

// ==========================================================================

namespace detail {

[[noreturn]] void fail_hex_char_to_byte(const char* message);

} // namespace detail

template<typename TByte, typename TChar = char>
inline constexpr TByte hex_char_to_byte(TChar hex_char)
{
	if (ascii::is_decimal(hex_char))
	{
		return static_cast<TByte>(hex_char - '0');
	}
	else if (hex_char >= 'a' && hex_char <= 'f')
	{
		return static_cast<TByte>(0xA + hex_char - 'a');
	}
	else if (hex_char >= 'A' && hex_char <= 'F')
	{
		return static_cast<TByte>(0xA + hex_char - 'A');
	}
	else
	{
		detail::fail_hex_char_to_byte("Invalid hex character.");
	}
}

// ==========================================================================

namespace detail {

[[noreturn]] void fail_hex_chars_to_bytes(const char* message);

} // namespace detail

template<typename TChar, typename TByte>
inline constexpr Int hex_chars_to_bytes(Span<TChar> chars, Span<TByte> bytes)
{
	const auto char_count = chars.get_size();
	const auto half_char_count = char_count / 2;
	const auto byte_count = bytes.get_size();

	if (byte_count < half_char_count)
	{
		detail::fail_hex_chars_to_bytes("Not enough bytes.");
	}

	if ((half_char_count * 2) != char_count)
	{
		detail::fail_hex_chars_to_bytes("Invalid chararacter count.");
	}

	auto char_index = decltype(char_count){};
	auto byte_index = decltype(byte_count){};

	while (true)
	{
		if (char_index == char_count || byte_index == byte_count)
		{
			break;
		}

		const auto high_nibble = hex_char_to_byte<TByte>(chars[char_index + 0]);
		const auto low_nibble = hex_char_to_byte<TByte>(chars[char_index + 1]);
		char_index += 2;
		const auto byte = static_cast<TByte>((high_nibble << 4) | low_nibble);
		bytes[byte_index] = byte;
		byte_index += 1;
	}

	return byte_index;
}

// ==========================================================================

namespace detail {

[[noreturn]] void fail_bytes_to_hex_chars(const char* message);

} // namespace detail

template<typename TByte, typename TChar>
inline constexpr Int bytes_to_hex_chars(Span<TByte> bytes, Span<TChar> chars)
{
	const auto byte_count = bytes.get_size();
	const auto char_count = chars.get_size();

	if (char_count < (byte_count * 2))
	{
		detail::fail_bytes_to_hex_chars("Not enough characters.");
	}

	auto byte_index = decltype(byte_count){};
	auto char_index = decltype(char_count){};

	while (true)
	{
		if (byte_index == byte_count || char_index == char_count)
		{
			break;
		}

		const auto byte = static_cast<unsigned int>(static_cast<std::make_unsigned_t<TByte>>(bytes[byte_index]));
		byte_index += 1;
		const auto char_1 = nibble_to_hex_char<TChar>((byte >> 4) & 0xF);
		const auto char_2 = nibble_to_hex_char<TChar>(byte & 0xF);
		chars[char_index + 0] = char_1;
		chars[char_index + 1] = char_2;
		char_index += 2;
	}

	return char_index;
}

// ==========================================================================

constexpr auto char_conv_min_base = 2;
constexpr auto char_conv_max_base = 36;

// --------------------------------------------------------------------------

enum class ToCharsFormat : unsigned int
{
	none = 0,

	// Prefix digits for common bases.
	// - "0b" or "0B" for base 2.
	// - "0" for base 8.
	// - "0x" or "0X" for base 16.
	prefix = 1U << 0,

	// Aways prefix a positive number with a plus sign.
	plus_sign = 1U << 1,

	// Uppercase prefix.
	uppercase_prefix = 1U << 2,

	// Uppercase digits.
	uppercase_value = 1U << 3,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(ToCharsFormat)

namespace detail {

[[noreturn]] void fail_to_chars(const char* message);

constexpr auto max_integral_bits = 64;
constexpr auto max_integral_prefix_size = 2;

struct ToCharsIntegralTag {};

template<typename T>
struct ToCharsIntegralToUnsigned
{
	using Unsigned = std::make_unsigned_t<T>;

	template<typename U = T, std::enable_if_t<std::is_signed<U>::value, int> = 0>
	constexpr Unsigned operator()(U value) const noexcept
	{
		return static_cast<Unsigned>((value >= 0 || value == std::numeric_limits<U>::min()) ? value : -value);
	}

	template<typename U = T, std::enable_if_t<std::is_unsigned<U>::value, int> = 0>
	constexpr Unsigned operator()(U value) const noexcept
	{
		return value;
	}
};

} // namespace detail

template<typename TValue, typename TChar>
inline constexpr Int to_chars(
	TValue value,
	Span<TChar> chars_span,
	int base,
	ToCharsFormat format = ToCharsFormat{})
{
	static_assert(std::is_integral<TValue>::value, "Expected an integral type.");

	if (base < char_conv_min_base || base > char_conv_max_base)
	{
		detail::fail_to_chars("Base out of range.");
	}

	constexpr auto not_enough_space_error_message = "Not enough space.";

	const auto is_minus_sign = (value < 0);
	const auto is_uppercase_value = ((format & ToCharsFormat::uppercase_value) != ToCharsFormat{});

	using Unsigned = std::make_unsigned_t<TValue>;
	auto u_value = detail::ToCharsIntegralToUnsigned<TValue>{}(value);

	auto chars_it = chars_span.begin();
	const auto chars_end = chars_span.end();

	while (true)
	{
		if (chars_it == chars_end)
		{
			detail::fail_to_chars(not_enough_space_error_message);
		}

		const auto next_value = u_value / base;
		const auto digit = u_value - (next_value * base);
		assert(digit < static_cast<decltype(digit)>(base));
		u_value = next_value;
		const auto digit_char = static_cast<TChar>(digit < 10 ? '0' + digit : 'a' + digit - 10);
		*(chars_it++) = (is_uppercase_value ? ascii::to_upper(digit_char) : digit_char);

		if (next_value == 0)
		{
			break;
		}
	}

	if ((format & ToCharsFormat::prefix) != ToCharsFormat{} && value != 0)
	{
		const auto is_uppercase_prefix = ((format & ToCharsFormat::uppercase_prefix) != ToCharsFormat{});

		switch (base)
		{
			case 2:
				if ((chars_it + 2) >= chars_end)
				{
					detail::fail_to_chars(not_enough_space_error_message);
				}

				*(chars_it++) = (is_uppercase_prefix ? 'B' : 'b');
				*(chars_it++) = '0';
				break;

			case 8:
				if (chars_it == chars_end)
				{
					detail::fail_to_chars(not_enough_space_error_message);
				}

				*(chars_it++) = '0';
				break;

			case 16:
				if ((chars_it + 2) >= chars_end)
				{
					detail::fail_to_chars(not_enough_space_error_message);
				}

				*(chars_it++) = (is_uppercase_prefix ? 'X' : 'x');
				*(chars_it++) = '0';
				break;

			default:
				break;
		}
	}

	auto sign = TChar{};

	if (is_minus_sign)
	{
		sign = '-';
	}
	else if ((format & ToCharsFormat::plus_sign) != ToCharsFormat{})
	{
		sign = '+';
	}

	if (sign != TChar{})
	{
		if (chars_it == chars_end)
		{
			detail::fail_to_chars(not_enough_space_error_message);
		}

		*(chars_it++) = sign;
	}

	const auto char_count = chars_it - chars_span.begin();
	auto left_chars = chars_span.begin();
	auto right_chars = chars_span.begin() + char_count - 1;
	const auto half_char_count = char_count / 2;

	for (auto i = decltype(half_char_count){}; i < half_char_count; ++i)
	{
		const auto temp = *left_chars;
		*(left_chars++) = *right_chars;
		*(right_chars--) = temp;
	}

	return char_count;
}

// ==========================================================================

enum class FromCharsFormat : unsigned int
{
	none = 0,

	// The characters does not contain the prefix.
	// Note, the base can not be zero.
	no_prefix = 1U << 0,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(FromCharsFormat)

namespace detail {

[[noreturn]] void fail_from_chars(const char* message);

template<typename T, typename TUnsigned>
struct FromCharsIntegralFromUnsigned
{
	template<typename U = T, std::enable_if_t<std::is_signed<U>::value, int> = 0>
	constexpr U operator()(TUnsigned u_value, bool is_negative) const noexcept
	{
		return is_negative ? -static_cast<U>(u_value) : static_cast<U>(u_value);
	}

	template<typename U = T, std::enable_if_t<std::is_unsigned<U>::value, int> = 0>
	constexpr U operator()(TUnsigned u_value, bool) const noexcept
	{
		return u_value;
	}
};

} // namespace detail

template<typename TValue, typename TChar>
inline constexpr TValue from_chars(
	Span<TChar> chars_span,
	int base = 0,
	FromCharsFormat format = FromCharsFormat{})
{
	if (chars_span.is_empty())
	{
		detail::fail_from_chars("Empty character sequence.");
	}

	if (base != 0 && (base < char_conv_min_base || base > char_conv_max_base))
	{
		detail::fail_from_chars("Base out of range.");
	}

	auto chars_it = chars_span.begin();
	const auto chars_end = chars_span.end();

	auto has_minus_sign = false;

	if (*chars_it == '-')
	{
		if (!std::is_signed<TValue>::value)
		{
			detail::fail_from_chars("Negative value for unsigned type.");
		}

		has_minus_sign = true;
		chars_it += 1;
	}
	else if (*chars_it == '+')
	{
		chars_it += 1;
	}

	auto detected_base = 0;

	if ((format & FromCharsFormat::no_prefix) == FromCharsFormat{})
	{
		if ((chars_it + 2) <= chars_end)
		{
			if (chars_it[0] == '0')
			{
				if (chars_it[1] == 'b' || chars_it[1] == 'B')
				{
					detected_base = 2;
					chars_it += 2;
				}
				else if (chars_it[1] == 'x' || chars_it[1] == 'X')
				{
					detected_base = 16;
					chars_it += 2;
				}
				else if (chars_it[1] != '0')
				{
					detected_base = 8;
					chars_it += 1;
				}
			}
		}

		if (detected_base == 0 && chars_it < chars_end)
		{
			detected_base = 10;
		}
	}

	if (base == 0)
	{
		if (detected_base == 0)
		{
			detail::fail_from_chars("Unable to detect a base.");
		}

		base = detected_base;
	}
	else if (detected_base != 0 && detected_base != base)
	{
		detail::fail_from_chars("Base mismatch.");
	}

	if (chars_it == chars_end)
	{
		detail::fail_from_chars("No digits.");
	}

	const auto max_value = (
		std::is_signed<TValue>::value ?
			(has_minus_sign ? std::numeric_limits<TValue>::min() : std::numeric_limits<TValue>::max()) :
		std::numeric_limits<TValue>::max());

	using Unsigned = std::make_unsigned_t<TValue>;
	const auto max_u_value = static_cast<Unsigned>(max_value);
	const auto max_prev_u_value = max_u_value / base;
	const auto max_last_digit = static_cast<TChar>(max_u_value - (max_prev_u_value * base));

	auto u_value = Unsigned{};

	while (chars_it != chars_end)
	{
		const auto digit_char = *chars_it++;
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
			detail::fail_from_chars("Invalid digit character.");
		}

		if (digit >= base)
		{
			detail::fail_from_chars("Digit character out of range.");
		}

		if (u_value > max_prev_u_value || (u_value == max_prev_u_value && digit > max_last_digit))
		{
			detail::fail_from_chars("Number overflow.");
		}

		u_value *= base;
		u_value += static_cast<Unsigned>(digit);
	}

	return detail::FromCharsIntegralFromUnsigned<TValue, Unsigned>{}(u_value, has_minus_sign);
}

} // namespace bstone

#endif // !BSTONE_CHAR_CONV_INCLUDED
