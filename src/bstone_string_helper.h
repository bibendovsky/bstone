/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


#ifndef BSTONE_STRING_HELPER_INCLUDED
#define BSTONE_STRING_HELPER_INCLUDED


#include <string>


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


} // bstone


#endif // !BSTONE_STRING_HELPER_INCLUDED
