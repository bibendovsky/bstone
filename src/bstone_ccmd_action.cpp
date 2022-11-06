/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_ccmd_action.h"

namespace bstone {

CCmdActionException::CCmdActionException(const char* message) noexcept
	:
	Exception{"BSTONE_CCMD_ACTION", message}
{}

// ==========================================================================

CCmdAction::CCmdAction() noexcept = default;
CCmdAction::~CCmdAction() = default;

void CCmdAction::invoke(CCmdActionArgs args)
try
{
	do_invoke(args);
}
catch (...)
{
	fail_nested(__func__);
}

void CCmdAction::operator()(CCmdActionArgs args)
try
{
	invoke(args);
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void CCmdAction::fail(const char* message)
{
	throw CCmdActionException{message};
}

[[noreturn]] void CCmdAction::fail_nested(const char* message)
{
	std::throw_with_nested(CCmdActionException{message});
}

} // namespace bstone
