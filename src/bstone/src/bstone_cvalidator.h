/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVALIDATOR_INCLUDED
#define BSTONE_CVALIDATOR_INCLUDED

#include <string_view>

namespace bstone {

class CValidator
{
public:
	static void validate_name(std::string_view name);
};

} // namespace bstone

#endif // !BSTONE_CVALIDATOR_INCLUDED
