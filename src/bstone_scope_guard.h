/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Calls specified callbacks on entering (optional) the scope and upon it's leaving.
//


#ifndef BSTONE_SCOPE_GUARD_INCLUDED
#define BSTONE_SCOPE_GUARD_INCLUDED


#include <functional>
#include <utility>


namespace bstone
{


class ScopeGuard
{
public:
	using Callback = std::function<void()>;


	ScopeGuard(
		Callback leave_callback)
		:
		leave_callback_{leave_callback}
	{
	}

	ScopeGuard(
		Callback enter_callback,
		Callback leave_callback)
		:
		ScopeGuard{leave_callback}
	{
		enter_callback();
	}

	ScopeGuard(
		const ScopeGuard& rhs) = delete;

	ScopeGuard(
		ScopeGuard&& rhs)
		:
		leave_callback_{std::move(rhs.leave_callback_)}
	{
	}

	ScopeGuard& operator=(
		const ScopeGuard& rhs) = delete;

	~ScopeGuard()
	{
		leave_callback_();
	}


private:
	Callback leave_callback_;
}; // ScopeGuard


} // bstone


#endif // !BSTONE_SCOPE_GUARD_INCLUDED
