/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CVALIDATOR_INCLUDED
#define BSTONE_CVALIDATOR_INCLUDED

#include "bstone_string_view.h"

namespace bstone {

class CValidator
{
public:
	static void validate_name(StringView name);

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
};

} // namespace bstone

#endif // !BSTONE_CVALIDATOR_INCLUDED
