/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
