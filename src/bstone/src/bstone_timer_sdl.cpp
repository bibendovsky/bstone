/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Timer (SDL)

#include "bstone_timer.h"
#include "bstone_exception.h"
#include "bstone_sdl.h"
#include "SDL3/SDL_timer.h"

namespace bstone {

class Timer::Impl
{
public:
	static Uint64 SDLCALL sdl_callback(void* userdata, SDL_TimerID timerID, Uint64 interval);
};

// --------------------------------------

Uint64 SDLCALL Timer::Impl::sdl_callback(void* userdata, [[maybe_unused]] SDL_TimerID timerID, Uint64 interval)
{
	Timer& timer = *static_cast<Timer*>(userdata);
	timer.callback_(timer.user_data_);
	return interval;
}

// ======================================

Timer::~Timer()
{
	stop();
}

bool Timer::started() const
{
	return native_handle_ != 0;
}

void Timer::start_ns(long long period_ns, TimerCallback callback, void* user_data)
try
{
	if (period_ns <= 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid period value.");
	}
	if (callback == nullptr)
	{
		BSTONE_THROW_STATIC_SOURCE("Null callback.");
	}
	if (started())
	{
		BSTONE_THROW_STATIC_SOURCE("Already started.");
	}
	user_data_ = user_data;
	callback_ = callback;
	native_handle_ = SDL_AddTimerNS(static_cast<Uint64>(period_ns), &Impl::sdl_callback, this);
	if (!started())
	{
		sdl::fail("SDL_AddTimerNS");
	}
}
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void Timer::stop()
{
	if (!started())
	{
		return;
	}
	if (!SDL_RemoveTimer(native_handle_))
	{
		sdl::fail("SDL_RemoveTimer");
	}
	native_handle_ = 0;
}

} // namespace bstone
