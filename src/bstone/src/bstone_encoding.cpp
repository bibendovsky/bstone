/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Encoding utils.

#include "bstone_encoding.h"
#include "bstone_exception.h"
#include "bstone_utf.h"

namespace bstone {

std::u16string u8_to_u16(const std::string& u8_string)
try {
	auto u16_string = std::u16string{};
	u16_string.resize(u8_string.size());

	const auto u16_begin = u16_string.begin();

	const auto u16_next = utf::u8_to_u16(
		u8_string.cbegin(),
		u8_string.cend(),
		u16_begin,
		u16_string.end());

	const auto count = u16_next - u16_begin;
	u16_string.resize(count);

	return u16_string;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string u16_to_u8(const std::u16string& u16_string)
try {
	auto u8_string = std::string{};
	u8_string.resize(3 * u16_string.size());

	const auto u8_begin = u8_string.begin();

	const auto u8_next = utf::u16_to_u8(
		u16_string.cbegin(),
		u16_string.cend(),
		u8_begin,
		u8_string.end());

	const auto count = u8_next - u8_begin;
	u8_string.resize(count);

	return u8_string;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

