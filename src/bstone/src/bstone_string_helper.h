/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_STRING_HELPER_INCLUDED
#define BSTONE_STRING_HELPER_INCLUDED

#include <cstdint>
#include <string>

namespace bstone {

class StringHelper
{
public:
	static std::string to_lower_ascii(const std::string& string);
	static std::string bytes_to_hex_string(const std::uint8_t* bytes, int byte_count);
};

} // namespace bstone

#endif // BSTONE_STRING_HELPER_INCLUDED
