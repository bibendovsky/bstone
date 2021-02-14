/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
