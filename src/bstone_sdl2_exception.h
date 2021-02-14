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
// Base exception.
//


#ifndef BSTONE_SDL2_EXCEPTION_INCLUDED
#define BSTONE_SDL2_EXCEPTION_INCLUDED


#include <type_traits>

#include "SDL_video.h"

#include "bstone_exception.h"


namespace bstone
{


class Sdl2Exception :
	public Exception
{
public:
	Sdl2Exception();
}; // Sdl2Exception


class Sdl2EnsureResult
{
private:
	template<typename T>
	struct HasOperatorBoolImpl
	{
		template<typename U>
		static auto test(U*) -> decltype(std::declval<U>().operator bool());

		template<typename>
		static auto test(...)->std::false_type;

		using type = typename std::is_same<bool, decltype(test<T>(nullptr))>::type;
	}; // HasOperatorBoolImpl

	template<typename T>
	struct HasOperatorBool :
		HasOperatorBoolImpl<T>::type
	{
	}; // HasOperatorBool


public:
	template<
		typename TInt,
		std::enable_if_t<std::is_same<TInt, int>::value, int> = 0
	>
		explicit Sdl2EnsureResult(
			const TInt sdl_result)
	{
		if (sdl_result != 0)
		{
			throw Sdl2Exception{};
		}
	}

	template<
		typename TPointer,
		std::enable_if_t<std::is_pointer<TPointer>::value, int> = 0
	>
		explicit Sdl2EnsureResult(
			const TPointer sdl_result)
	{
		if (sdl_result == nullptr)
		{
			throw Sdl2Exception{};
		}
	}

	template<
		typename TBoolable,
		std::enable_if_t<HasOperatorBool<TBoolable>::value, int> = 0
	>
	explicit Sdl2EnsureResult(
		const TBoolable& sdl_result)
	{
		if (!sdl_result)
		{
			throw Sdl2Exception{};
		}
	}
}; // Sdl2EnsureResult


} // bstone


#endif // !BSTONE_SDL2_EXCEPTION_INCLUDED
