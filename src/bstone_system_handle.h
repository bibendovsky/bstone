/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// RAII wrapper for system handle.
//


#ifndef BSTONE_SYSTEM_HANDLE_INCLUDED
#define BSTONE_SYSTEM_HANDLE_INCLUDED


#include <algorithm>


namespace bstone
{


template<
	typename THandle,
	THandle TDefaultValue,
	void (*TDeleter)(const THandle native_handle) noexcept>
class SystemHandle
{
public:
	using Handle = THandle;


	SystemHandle() noexcept
		:
		handle_{get_default()}
	{
	}

	explicit SystemHandle(
		const Handle handle) noexcept
		:
		handle_{handle}
	{
	}

	SystemHandle(
		const SystemHandle& rhs) = delete;

	SystemHandle(
		SystemHandle&& rhs) noexcept
		:
		handle_{rhs.handle_}
	{
		rhs.handle_ = get_default();
	}

	SystemHandle& operator=(
		const SystemHandle& rhs) = delete;

	SystemHandle& operator=(
		SystemHandle&& rhs) noexcept
	{
		std::swap(handle_, rhs.handle_);

		return *this;
	}

	~SystemHandle()
	{
		close();
	}


	explicit operator bool() const noexcept
	{
		return handle_ != get_default();
	}


	bool is_default() const noexcept
	{
		return handle_ == get_default();
	}

	Handle get() const noexcept
	{
		return handle_;
	}

	void reset()
	{
		close();
	}

	void reset(
		const Handle handle)
	{
		close();

		handle_ = handle;
	}


	static Handle get_default() noexcept
	{
		return TDefaultValue;
	}


private:
	Handle handle_;


	void close() noexcept
	{
		const auto default_handle = get_default();

		if (handle_ == default_handle)
		{
			return;
		}

		auto handle = handle_;
		handle_ = default_handle;

		TDeleter(handle);
	}
}; // Handle


} // bstone


#endif // !BSTONE_SYSTEM_HANDLE_INCLUDED
