/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_STRING_HELPER_INCLUDED
#define BSTONE_STRING_HELPER_INCLUDED


#include <cstdint>
#include <string>

#include "bstone_array.h"
#include "bstone_char_conv.h"

namespace bstone
{


struct StringHelper final
{
public:
	static std::string to_lower_ascii(
		const std::string& string);


	static bool string_to_int(
		const std::string& string,
		int& int_value);

	static bool string_to_int16(
		const std::string& string,
		std::int16_t& int16_value);

	static bool string_to_uint16(
		const std::string& string,
		std::uint16_t& uint16_value);


	static std::string octet_to_hex_string(
		const int octet);


	template<
		typename T
	>
	static std::string make_left_padded_with_zero(
		T value,
		int max_length)
	{
		auto string = std::to_string(value);
		const auto pad_size = max_length - static_cast<int>(string.size());

		if (pad_size > 0)
		{
			string.insert(0, pad_size, '0');
		}

		return string;
	}


	const std::string& get_empty() const;
}; // StringHelper


template<typename TItem, std::intptr_t TSize>
inline std::string array_to_hex_string(const Array<TItem, TSize>& array)
{
	auto string = std::string(TSize * 2, '\0');

	bytes_to_hex_chars(
		array.cbegin(),
		array.cend(),
		&string[0],
		&string[0] + string.size());

	return string;
}

} // bstone


#endif // !BSTONE_STRING_HELPER_INCLUDED
