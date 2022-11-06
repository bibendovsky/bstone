/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_CCMD_ACTION_INCLUDED
#define BSTONE_CCMD_ACTION_INCLUDED

#include "bstone_exception.h"
#include "bstone_span.h"
#include "bstone_string_view.h"

namespace bstone {

using CCmdActionArgs = Span<const StringView>;

// ==========================================================================

class CCmdActionException : public Exception
{
public:
	explicit CCmdActionException(const char* message) noexcept;
};

// ==========================================================================

class CCmdAction
{
public:
	CCmdAction() noexcept;
	virtual ~CCmdAction();

	void invoke(CCmdActionArgs args);
	void operator()(CCmdActionArgs args);

private:
	virtual void do_invoke(CCmdActionArgs args) = 0;

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
};

} // namespace bstone

#endif // !BSTONE_CCMD_ACTION_INCLUDED
