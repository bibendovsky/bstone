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


#include "bstone_string_helper.h"


[[noreturn]]
void Quit(
	const std::string& message);


namespace bstone
{


std::string StringHelper::to_lower_ascii(
	const std::string& string)
{
	if (string.empty())
	{
		return {};
	}

	auto string_lc = string;

	for (auto& char_lc : string_lc)
	{
		if (char_lc >= 'A' && char_lc <= 'Z')
		{
			char_lc = static_cast<char>('a' + char_lc - 'A');
		}
	}

	return string_lc;
}

bool StringHelper::string_to_int(
	const std::string& string,
	int& int_value)
{
	int_value = 0;

	if (string.empty())
	{
		return false;
	}

	try
	{
		int_value = std::stoi(string);
	}
	catch (std::exception&)
	{
		return false;
	}

	return true;
}

bool StringHelper::string_to_int16(
	const std::string& string,
	std::int16_t& int16_value)
{
	int16_value = 0;

	int int_value;

	if (!string_to_int(string, int_value))
	{
		return false;
	}

	if (int_value < -32'768 || int_value > 32'767)
	{
		return false;
	}

	int16_value = static_cast<std::int16_t>(int_value);

	return true;
}

bool StringHelper::string_to_uint16(
	const std::string& string,
	std::uint16_t& uint16_value)
{
	uint16_value = 0;

	int int_value;

	if (!string_to_int(string, int_value))
	{
		return false;
	}

	if (int_value < 0 || int_value > 65'535)
	{
		return false;
	}

	uint16_value = static_cast<std::uint16_t>(int_value);

	return true;
}

std::string StringHelper::octet_to_hex_string(
	const int octet)
{
	if (octet < 0 || octet > 0xFF)
	{
		Quit("Octet value out of range: " + std::to_string(octet) + ".");
	}

	const auto high_nibble = (octet >> 4) & 0xF;
	const auto low_nibble = (octet >> 0) & 0xF;

	const auto high_nibble_char = (
		high_nibble < 0xA ?
			static_cast<char>('0' + high_nibble) :
			static_cast<char>('A' + high_nibble - 0xA));

	const auto low_nibble_char = (
		low_nibble < 0xA ?
			static_cast<char>('0' + low_nibble) :
			static_cast<char>('A' + low_nibble - 0xA));

	const auto level_number_string = std::string{high_nibble_char, low_nibble_char};

	return level_number_string;
}

const std::string& StringHelper::get_empty() const
{
	static const auto result = std::string{};

	return result;
}


} // bstone
