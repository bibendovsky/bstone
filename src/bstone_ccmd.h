/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CCMD_INCLUDED
#define BSTONE_CCMD_INCLUDED

#include "bstone_ccmd_action.h"
#include "bstone_string_view.h"

namespace bstone {

class CCmd
{
public:
	CCmd(StringView name, CCmdAction& action);

	StringView get_name() const noexcept;
	CCmdAction& get_action() const noexcept;

private:
	StringView name_{};
	CCmdAction* action_{};

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
};

} // namespace bstone

#endif // !BSTONE_CCMD_INCLUDED
