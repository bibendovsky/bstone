/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ccmd.h"
#include "bstone_cvalidator.h"
#include "bstone_exception.h"

namespace bstone {

CCmd::CCmd(StringView name, CCmdAction& action)
try
	:
	name_{name},
	action_{&action}
{
	CValidator::validate_name(name_);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

StringView CCmd::get_name() const noexcept
{
	return name_;
}

CCmdAction& CCmd::get_action() const noexcept
{
	return *action_;
}

} // namespace bstone
