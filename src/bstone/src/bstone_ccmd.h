/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CCMD_INCLUDED
#define BSTONE_CCMD_INCLUDED

#include "bstone_ccmd_action.h"
#include <string_view>

namespace bstone {

class CCmd
{
public:
	CCmd(std::string_view name, CCmdAction& action);

	std::string_view get_name() const noexcept;
	CCmdAction& get_action() const noexcept;

private:
	std::string_view name_{};
	CCmdAction* action_{};
};

} // namespace bstone

#endif // !BSTONE_CCMD_INCLUDED
