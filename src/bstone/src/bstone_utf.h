/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Unicode transformation primitives.

#ifndef BSTONE_UTF_INCLUDED
#define BSTONE_UTF_INCLUDED

#include <cstdint>

#include "bstone_exception.h"
#include "bstone_unicode.h"
#include "bstone_utf16.h"
#include "bstone_utf8.h"

namespace bstone {
namespace utf {

template<typename TU8CodeUnitIter>
inline std::intptr_t get_u8_to_u16_size(
	TU8CodeUnitIter utf8_code_units_begin,
	TU8CodeUnitIter utf8_code_units_end)
{
	auto utf16_code_units_size = std::intptr_t{};
	auto code_point = unicode::CodePoint{};
	auto utf8_code_units_iter = utf8_code_units_begin;

	while (utf8_code_units_iter != utf8_code_units_end)
	{
		utf8::decode_code_point(utf8_code_units_iter, utf8_code_units_end, utf8_code_units_iter, code_point);
		utf16_code_units_size += std::intptr_t{1} + (code_point >= utf16::min_code_unit_2);
	}

	return utf16_code_units_size;
}

// ==========================================================================

template<typename TU8CodeUnitIter, typename TU16CodeUnitIter>
inline TU16CodeUnitIter u8_to_u16(
	TU8CodeUnitIter utf8_code_units_begin,
	TU8CodeUnitIter utf8_code_units_end,
	TU16CodeUnitIter utf16_code_units_begin,
	TU16CodeUnitIter utf16_code_units_end)
{
	auto code_point = unicode::CodePoint{};
	auto utf8_code_units_iter = utf8_code_units_begin;
	auto utf16_code_units_iter = utf16_code_units_begin;

	while (utf8_code_units_iter != utf8_code_units_end)
	{
		utf8::decode_code_point(utf8_code_units_iter, utf8_code_units_end, utf8_code_units_iter, code_point);
		utf16::encode_code_point(code_point, utf16_code_units_iter, utf16_code_units_end, utf16_code_units_iter);
	}

	return utf16_code_units_iter;
}

// ==========================================================================

template<typename TU8CodeUnitIter>
inline std::intptr_t get_u8_to_u32_size(
	TU8CodeUnitIter utf8_code_units_begin,
	TU8CodeUnitIter utf8_code_units_end)
{
	auto utf32_code_units_size = std::intptr_t{};
	auto code_point = unicode::CodePoint{};
	auto utf8_code_units_iter = utf8_code_units_begin;

	while (utf8_code_units_iter != utf8_code_units_end)
	{
		utf8::decode_code_point(utf8_code_units_iter, utf8_code_units_end, utf8_code_units_iter, code_point);
		utf32_code_units_size += 1;
	}

	return utf32_code_units_size;
}

// ==========================================================================

template<typename TU8CodeUnitIter, typename TU32CodeUnitIter>
inline TU32CodeUnitIter u8_to_u32(
	TU8CodeUnitIter utf8_code_units_begin,
	TU8CodeUnitIter utf8_code_units_end,
	TU32CodeUnitIter utf32_code_units_begin,
	TU32CodeUnitIter utf32_code_units_end)
{
	using U32CodeUnit = std::decay_t<decltype(*utf32_code_units_begin)>;

	auto utf8_code_units_iter = utf8_code_units_begin;
	auto utf32_code_units_iter = utf32_code_units_begin;

	auto code_point = U32CodeUnit{};

	while (utf8_code_units_iter != utf8_code_units_end)
	{
		utf8::decode_code_point(utf8_code_units_iter, utf8_code_units_end, utf8_code_units_iter, code_point);

		if (utf32_code_units_iter == utf32_code_units_end)
		{
			BSTONE_THROW_STATIC_SOURCE("Destination buffer too small.");
		}

		*utf32_code_units_iter = code_point;
		++utf32_code_units_iter;
	}

	return utf32_code_units_iter;
}

// ==========================================================================

template<typename TU16CodeUnitIter>
inline std::intptr_t get_u16_to_u8_size(
	TU16CodeUnitIter utf16_code_units_begin,
	TU16CodeUnitIter utf16_code_units_end)
{
	auto utf8_code_units_size = std::intptr_t{};
	auto code_point = unicode::CodePoint{};
	auto utf16_code_units_iter = utf16_code_units_begin;

	while (utf16_code_units_iter != utf16_code_units_end)
	{
		utf16::decode_code_point(utf16_code_units_iter, utf16_code_units_end, utf16_code_units_iter, code_point);
		utf8_code_units_size += 1;

		if (code_point <= utf8::byte_1_max_range)
		{
		}
		else if (code_point >= utf8::byte_2_min_range && code_point <= utf8::byte_2_max_range)
		{
			utf8_code_units_size += 1;
		}
		else if (code_point >= utf8::byte_3_min_range && code_point <= utf8::byte_3_max_range)
		{
			utf8_code_units_size += 2;
		}
		else
		{
			utf8_code_units_size += 3;
		}
	}

	return utf8_code_units_size;
}

// ==========================================================================

template<typename TU16CodeUnitIter, typename TU8CodeUnitIter>
inline TU8CodeUnitIter u16_to_u8(
	TU16CodeUnitIter utf16_code_units_begin,
	TU16CodeUnitIter utf16_code_units_end,
	TU8CodeUnitIter utf8_code_units_begin,
	TU8CodeUnitIter utf8_code_units_end)
{
	auto code_point = unicode::CodePoint{};
	auto utf16_code_units_iter = utf16_code_units_begin;
	auto utf8_code_units_iter = utf8_code_units_begin;

	while (utf16_code_units_iter != utf16_code_units_end)
	{
		utf16::decode_code_point(utf16_code_units_iter, utf16_code_units_end, utf16_code_units_iter, code_point);
		utf8::encode_code_point(code_point, utf8_code_units_iter, utf8_code_units_end, utf8_code_units_iter);
	}

	return utf8_code_units_iter;
}

} // namespace utf
} // namespace bstone

#endif // BSTONE_UTF_INCLUDED
