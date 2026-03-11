/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_HELPER_INCLUDED
#define BSTONE_STRING_HELPER_INCLUDED

#include "bstone_char_conv.h"
#include <array>
#include <string>

namespace bstone {

class StringHelper
{
public:
	static std::string to_lower_ascii(const std::string& string);

	template<typename TByte, std::size_t TSize>
	static std::string array_to_hex_string(const std::array<TByte, TSize>& array)
	{
		std::string string(TSize * 2, '\0');
		bytes_to_hex_chars(
			array.data(),
			array.data() + array.size(),
			&string[0],
			&string[0] + string.size());
		return string;
	}
};

} // namespace bstone

#endif // BSTONE_STRING_HELPER_INCLUDED
