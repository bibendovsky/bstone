/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Universally unique identifier (UUID).
//
// Supported formats:
// - "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
// - "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
// - "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"

#ifndef BSTONE_UUID_INCLUDED
#define BSTONE_UUID_INCLUDED

#include <cstdint>

#include "bstone_array.h"
#include "bstone_ascii.h"
#include "bstone_char_conv.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_string_view.h"
#include "bstone_utility.h"

namespace bstone {

constexpr auto uuid_value_size = 16;

enum class UuidStringFormat
{
	none,

	// "AAAAAAAABBBBCCCCDDDDEEEEEEEEEEEE"
	without_hyphens,

	// "AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE"
	with_hyphens,

	// "{AAAAAAAA-BBBB-CCCC-DDDD-EEEEEEEEEEEE}"
	with_hyphens_and_braces,
};

enum class UuidStringCase
{
	none,
	lower,
	upper,
};

enum class UuidEndianType
{
	none,
	big,
	little_mixed,
};

using UuidValue = Array<std::uint8_t, uuid_value_size>;

// ==========================================================================

class Uuid
{
public:
	constexpr Uuid() noexcept = default;

	constexpr Uuid(const UuidValue& value) noexcept
		:
		value_{value}
	{}

	constexpr Uuid(
		std::uint32_t a, // group 1
		std::uint16_t b, // group 2
		std::uint16_t c, // group 3
		std::uint8_t d, // group 4
		std::uint8_t e, // group 4
		std::uint8_t f, // group 5
		std::uint8_t g, // group 5
		std::uint8_t h, // group 5
		std::uint8_t i, // group 5
		std::uint8_t j, // group 5
		std::uint8_t k // group 5
	) noexcept
		:
		value_{
			static_cast<std::uint8_t>(a >> 24),
			static_cast<std::uint8_t>(a >> 16),
			static_cast<std::uint8_t>(a >> 8),
			static_cast<std::uint8_t>(a >> 0),
			static_cast<std::uint8_t>(b >> 8),
			static_cast<std::uint8_t>(b >> 0),
			static_cast<std::uint8_t>(c >> 8),
			static_cast<std::uint8_t>(c >> 0),
			d, e, f, g, h, i, j, k}
	{}

	constexpr Uuid(
		std::uint32_t a, // group 1
		std::uint16_t b, // group 2
		std::uint16_t c, // group 3
		const std::uint8_t (&d)[8] // group 4 and 5
	) noexcept
		:
		value_{
			static_cast<std::uint8_t>(a >> 24),
			static_cast<std::uint8_t>(a >> 16),
			static_cast<std::uint8_t>(a >> 8),
			static_cast<std::uint8_t>(a >> 0),
			static_cast<std::uint8_t>(b >> 8),
			static_cast<std::uint8_t>(b >> 0),
			static_cast<std::uint8_t>(c >> 8),
			static_cast<std::uint8_t>(c >> 0),
			d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]}
	{}

	constexpr explicit Uuid(const std::uint8_t (&value)[uuid_value_size]) noexcept
		:
		value_{
			value[0], value[1], value[2], value[3],
			value[4], value[5], value[6], value[7],
			value[8], value[9], value[10], value[11],
			value[12], value[13], value[14], value[15]}
	{}

	template<typename TChar>
	constexpr explicit Uuid(BasicStringView<TChar> string_view)
	{
		switch (string_view.get_size())
		{
			case 32: parse_without_hyphens(string_view); break;
			case 36: parse_with_hyphens(string_view); break;
			case 38: parse_with_hyphens_and_braces(string_view); break;
			default: BSTONE_THROW_STATIC_SOURCE("Unsupported string length.");
		}
	}

	template<typename TChar>
	constexpr explicit Uuid(const TChar* string, std::intptr_t length)
		:
		Uuid{BasicStringView<TChar>{string, length}}
	{}

	template<typename TChar>
	constexpr explicit Uuid(const TChar* string)
		:
		Uuid{BasicStringView<TChar>{string}}
	{}

	constexpr bool is_nil() const noexcept
	{
		return
			value_[0] == 0 &&
			value_[1] == 0 &&
			value_[2] == 0 &&
			value_[3] == 0 &&
			value_[4] == 0 &&
			value_[5] == 0 &&
			value_[6] == 0 &&
			value_[7] == 0 &&
			value_[8] == 0 &&
			value_[9] == 0 &&
			value_[10] == 0 &&
			value_[11] == 0 &&
			value_[12] == 0 &&
			value_[13] == 0 &&
			value_[14] == 0 &&
			value_[15] == 0;
	}

	constexpr const UuidValue& get_value() const noexcept
	{
		return value_;
	}

	constexpr UuidValue& get_value() noexcept
	{
		return value_;
	}

	template<typename TChar>
	constexpr TChar* to_chars(
		TChar* chars_begin,
		TChar* chars_end,
		UuidStringFormat string_format = UuidStringFormat::with_hyphens,
		UuidStringCase string_case = UuidStringCase::lower) const
	{
		auto case_func = CaseFunc<TChar>{};

		switch (string_case)
		{
			case UuidStringCase::lower: case_func = no_case_func<TChar>; break;
			case UuidStringCase::upper: case_func = upper_case_func<TChar>; break;
			default: BSTONE_THROW_STATIC_SOURCE("Unknown string case.");
		}

		switch (string_format)
		{
			case UuidStringFormat::without_hyphens:
				return to_chars_without_hyphens(case_func, chars_begin, chars_end);

			case UuidStringFormat::with_hyphens:
				return to_chars_with_hyphens(case_func, chars_begin, chars_end);

			case UuidStringFormat::with_hyphens_and_braces:
				return to_chars_with_hyphens_and_braces(case_func, chars_begin, chars_end);

			default: BSTONE_THROW_STATIC_SOURCE("Unknown string format.");
		}
	}

	constexpr void swap_bytes(UuidEndianType endian_type)
	{
		switch (endian_type)
		{
			case UuidEndianType::big:
				endian::swap_byte_array(value_.get_data(), value_.get_size());
				break;

			case UuidEndianType::little_mixed:
				// Group 1.
				bstone::swop(value_[0], value_[3]);
				bstone::swop(value_[1], value_[2]);

				// Group 2.
				bstone::swop(value_[4], value_[5]);

				// Group 3.
				bstone::swop(value_[6], value_[7]);

				break;

			default: BSTONE_THROW_STATIC_SOURCE("Unknown endian type.");
		}
	}

	static Uuid generate();

private:
	UuidValue value_{};

private:
	template<typename TChar>
	using CaseFunc = void (*)(TChar* chars_begin, TChar* chars_end);

	template<typename TChar>
	constexpr static void no_case_func(TChar*, TChar*) noexcept {}

	template<typename TChar>
	constexpr static void upper_case_func(TChar* chars_begin, TChar* chars_end) noexcept
	{
		ascii::to_upper(chars_begin, chars_end);
	}

	template<typename TChar>
	constexpr TChar* to_chars_without_hyphens(
		CaseFunc<TChar> case_func,
		TChar* chars_begin,
		TChar* chars_end) const
	{
		if (chars_begin + 32 != chars_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
		}

		const auto uuid_chars_end = bstone::bytes_to_hex_chars(
			value_.cbegin(), value_.cend(), chars_begin, chars_end);
		case_func(chars_begin, uuid_chars_end);
		return uuid_chars_end;
	}

	template<typename TChar>
	constexpr TChar* to_chars_with_hyphens(
		CaseFunc<TChar> case_func,
		TChar* chars_begin,
		TChar* chars_end) const
	{
		const auto end_it = chars_begin + 36;

		if (chars_end < end_it)
		{
			BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
		}

		const auto bytes = value_.get_data();

		bstone::bytes_to_hex_chars(bytes + 0, bytes + 4, chars_begin + 0, chars_begin + 8);
		chars_begin[8] = '-';
		bstone::bytes_to_hex_chars(bytes + 4, bytes + 6, chars_begin + 9, chars_begin + 13);
		chars_begin[13] = '-';
		bstone::bytes_to_hex_chars(bytes + 6, bytes + 8, chars_begin + 14, chars_begin + 18);
		chars_begin[18] = '-';
		bstone::bytes_to_hex_chars(bytes + 8, bytes + 10, chars_begin + 19, chars_begin + 23);
		chars_begin[23] = '-';
		bstone::bytes_to_hex_chars(bytes + 10, bytes + 16, chars_begin + 24, chars_begin + 36);

		case_func(chars_begin, end_it);

		return end_it;
	}

	template<typename TChar>
	constexpr TChar* to_chars_with_hyphens_and_braces(
		CaseFunc<TChar> case_func,
		TChar* chars_begin,
		TChar* chars_end) const
	{
		const auto end_it = chars_begin + 38;

		if (chars_end < end_it)
		{
			BSTONE_THROW_STATIC_SOURCE("Buffer too small.");
		}

		const auto bytes = value_.get_data();

		chars_begin[0] = '{';
		bstone::bytes_to_hex_chars(bytes + 0, bytes + 4, chars_begin + 1, chars_begin + 9);
		chars_begin[9] = '-';
		bstone::bytes_to_hex_chars(bytes + 4, bytes + 6, chars_begin + 10, chars_begin + 14);
		chars_begin[14] = '-';
		bstone::bytes_to_hex_chars(bytes + 6, bytes + 8, chars_begin + 15, chars_begin + 19);
		chars_begin[19] = '-';
		bstone::bytes_to_hex_chars(bytes + 8, bytes + 10, chars_begin + 20, chars_begin + 24);
		chars_begin[24] = '-';
		bstone::bytes_to_hex_chars(bytes + 10, bytes + 16, chars_begin + 25, chars_begin + 37);
		chars_begin[37] = '}';

		case_func(chars_begin, end_it);

		return end_it;
	}

	template<typename TChar>
	constexpr void parse_without_hyphens(BasicStringView<TChar> string_view)
	{
		bstone::hex_chars_to_bytes(string_view.cbegin(), string_view.cend(), value_.begin(), value_.end());
	}

	template<typename TChar>
	constexpr void parse_with_hyphens(BasicStringView<TChar> string_view)
	{
		const auto chars = string_view.get_data();

		if (chars[8] != '-' || chars[13] != '-' || chars[18] != '-' || chars[23] != '-')
		{
			BSTONE_THROW_STATIC_SOURCE("Expected hyphens.");
		}

		const auto bytes = value_.get_data();

		bstone::hex_chars_to_bytes(chars + 0, chars + 8, bytes + 0, bytes + 4);
		bstone::hex_chars_to_bytes(chars + 9, chars + 13, bytes + 4, bytes + 6);
		bstone::hex_chars_to_bytes(chars + 14, chars + 18, bytes + 6, bytes + 8);
		bstone::hex_chars_to_bytes(chars + 19, chars + 23, bytes + 8, bytes + 10);
		bstone::hex_chars_to_bytes(chars + 24, chars + 36, bytes + 10, bytes + 16);
	}

	template<typename TChar>
	constexpr void parse_with_hyphens_and_braces(BasicStringView<TChar> string_view)
	{
		const auto chars = string_view.get_data();

		if (chars[0] != '{' ||
			chars[9] != '-' || chars[14] != '-' || chars[19] != '-' || chars[24] != '-' ||
			chars[37] != '}')
		{
			BSTONE_THROW_STATIC_SOURCE("Expected hyphens and braces.");
		}

		const auto bytes = value_.get_data();

		bstone::hex_chars_to_bytes(chars + 1, chars + 9, bytes + 0, bytes + 4);
		bstone::hex_chars_to_bytes(chars + 10, chars + 14, bytes + 4, bytes + 6);
		bstone::hex_chars_to_bytes(chars + 15, chars + 19, bytes + 6, bytes + 8);
		bstone::hex_chars_to_bytes(chars + 20, chars + 24, bytes + 8, bytes + 10);
		bstone::hex_chars_to_bytes(chars + 25, chars + 37, bytes + 10, bytes + 16);
	}
};

static_assert(sizeof(Uuid) == 16, "Invalid Uuid class size.");

// ==========================================================================

constexpr inline bool operator==(const Uuid& lhs, const Uuid& rhs) noexcept
{
	return lhs.get_value() == rhs.get_value();
}

constexpr inline bool operator!=(const Uuid& lhs, const Uuid& rhs) noexcept
{
	return !(lhs == rhs);
}

} // namespace bstone

#endif // BSTONE_UUID_INCLUDED
