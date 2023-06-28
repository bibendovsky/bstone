/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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


#ifdef _USING_V110_SDK71_
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
BSTONE_BEGIN_FUNC_TRY
#ifdef _USING_V110_SDK71_
	const auto& uint16_string = detail::get_wstring_converter().from_bytes(utf8_string);

	return std::u16string(
		reinterpret_cast<const char16_t*>(uint16_string.c_str()),
		uint16_string.size()
	);
#else
	return detail::get_wstring_converter().from_bytes(utf8_string);
#endif // _USING_V110_SDK71_
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

std::string utf16_to_utf8(
	const std::u16string& utf16_string)
BSTONE_BEGIN_FUNC_TRY
#ifdef _USING_V110_SDK71_
		const auto& uint16_string = std::basic_string<std::uint16_t>(
			reinterpret_cast<const std::uint16_t*>(utf16_string.c_str()),
			utf16_string.size()
		);

		return detail::get_wstring_converter().to_bytes(uint16_string);
#else
		return detail::get_wstring_converter().to_bytes(utf16_string);
#endif // _USING_V110_SDK71_
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED


} // bstone

