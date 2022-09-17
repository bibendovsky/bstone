/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Base exception.
//


#ifndef BSTONE_SDL_EXCEPTION_INCLUDED
#define BSTONE_SDL_EXCEPTION_INCLUDED


#include <utility>


namespace bstone
{


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

[[noreturn]]
void fail_sdl();

void ensure_sdl_result(
	int sdl_result);

template<
	typename T
>
T* ensure_sdl_result(
	T* sdl_result)
{
	if (!sdl_result)
	{
		fail_sdl();
	}

	return sdl_result;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

class SdlEnsureResult
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
		explicit SdlEnsureResult(
			const TInt sdl_result)
	{
		if (sdl_result != 0)
		{
			fail_sdl();
		}
	}

	template<
		typename TPointer,
		std::enable_if_t<std::is_pointer<TPointer>::value, int> = 0
	>
		explicit SdlEnsureResult(
			const TPointer sdl_result)
	{
		if (!sdl_result)
		{
			fail_sdl();
		}
	}

	template<
		typename TBoolable,
		std::enable_if_t<HasOperatorBool<TBoolable>::value, int> = 0
	>
	explicit SdlEnsureResult(
		const TBoolable& sdl_result)
	{
		if (!sdl_result)
		{
			fail_sdl();
		}
	}
}; // SdlEnsureResult

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // bstone


#endif // !BSTONE_SDL_EXCEPTION_INCLUDED
