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


//
// Encoding utils.
//



#include "bstone_encoding.h"

#include <cstdint>

#include <codecvt>
#include <exception>
#include <locale>

#include "bstone_exception.h"


namespace bstone
{


namespace detail
{


class Utf8ToUtf16Exception :
	public Exception
{
public:
	explicit Utf8ToUtf16Exception(
		const std::exception& std_exception)
		:
		Exception{std::string{"[UTF8_TO_UTF16] "} + std_exception.what()}
	{
	}
}; // Utf8ToUtf16Exception

class Utf16ToUtf8Exception :
	public Exception
{
public:
	explicit Utf16ToUtf8Exception(
		const std::exception& std_exception)
		:
		Exception{std::string{"[UTF16_TO_UTF8] "} + std_exception.what()}
	{
	}
}; // Utf16ToUtf8Exception


#if _USING_V110_SDK71_
using WStringConverter = std::wstring_convert<std::codecvt_utf8_utf16<std::uint16_t>, std::uint16_t>;
#else
using WStringConverter = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>;
#endif // _USING_V110_SDK71_


WStringConverter& get_wstring_converter()
{
	static WStringConverter result{};

	return result;
}


} // detail


std::u16string utf8_to_utf16(
	const std::string& utf8_string)
{
	try
	{
#ifdef _USING_V110_SDK71_
		const auto& uint16_string = detail::get_wstring_converter().from_bytes(utf8_string);

		return std::u16string(
			reinterpret_cast<const char16_t*>(uint16_string.c_str()),
			uint16_string.size()
		);
#else
		return detail::get_wstring_converter().from_bytes(utf8_string);
#endif // _USING_V110_SDK71_
	}
	catch (const std::exception& ex)
	{
		throw detail::Utf8ToUtf16Exception{ex};
	}
}

std::string utf16_to_utf8(
	const std::u16string& utf16_string)
{
	try
	{
#if _USING_V110_SDK71_
		const auto& uint16_string = std::basic_string<std::uint16_t>(
			reinterpret_cast<const std::uint16_t*>(utf16_string.c_str()),
			utf16_string.size()
		);

		return detail::get_wstring_converter().to_bytes(uint16_string);
#else
		return detail::get_wstring_converter().to_bytes(utf16_string);
#endif // _USING_V110_SDK71_
	}
	catch (const std::exception& ex)
	{
		throw detail::Utf16ToUtf8Exception{ex};
	}
}


} // bstone

