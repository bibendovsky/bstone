/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_string_helper.h"
#include "bstone_ascii.h"
#include <algorithm>

namespace bstone {

std::string StringHelper::to_lower_ascii(const std::string& string)
{
	std::string result{};
	result.resize(string.size());
	std::transform(string.cbegin(), string.cend(), result.begin(), [](char ch){return ascii::to_lower(ch);});
	return result;
}

} // namespace bstone
