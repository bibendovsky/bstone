/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ccmd.h"
#include "bstone_cvalidator.h"
#include "bstone_exception.h"

namespace bstone {

class CCmdException : public Exception
{
public:
	explicit CCmdException(const char* message) noexcept
		:
		Exception{"BSTONE_CCMD", message}
	{}
};

// ==========================================================================

CCmd::CCmd(StringView name, CCmdAction& action)
try
	:
	name_{name},
	action_{&action}
{
	CValidator::validate_name(name_);
}
catch (...)
{
	fail_nested(__func__);
}

StringView CCmd::get_name() const noexcept
{
	return name_;
}

CCmdAction& CCmd::get_action() const noexcept
{
	return *action_;
}

[[noreturn]] void CCmd::fail(const char* message)
{
	throw CCmdException{message};
}

[[noreturn]] void CCmd::fail_nested(const char* message)
{
	std::throw_with_nested(CCmdException{message});
}

} // namespace bstone
