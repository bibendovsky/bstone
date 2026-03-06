/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Date and time management (SDL)

#include "bstone_sys_time.h"
#include "bstone_exception.h"
#include "bstone_sdl.h"
#include <cstddef>
#include "SDL3/SDL_time.h"

namespace bstone::sys {

TimeNs get_current_time_ns()
{
	SDL_Time sdl_time;
	if (!SDL_GetCurrentTime(&sdl_time))
	{
		sdl::fail("SDL_GetCurrentTime");
	}
	return sdl_time;
}

DateTime time_ns_to_date_time(TimeNs time_ns, DateTimeKind date_time_kind)
{
	static_assert(
		sizeof(DateTime) == sizeof(SDL_DateTime) &&
			offsetof(DateTime, year) == offsetof(SDL_DateTime, year) &&
			offsetof(DateTime, month) == offsetof(SDL_DateTime, month) &&
			offsetof(DateTime, day) == offsetof(SDL_DateTime, day) &&
			offsetof(DateTime, hour) == offsetof(SDL_DateTime, hour) &&
			offsetof(DateTime, minute) == offsetof(SDL_DateTime, minute) &&
			offsetof(DateTime, second) == offsetof(SDL_DateTime, second) &&
			offsetof(DateTime, nanosecond) == offsetof(SDL_DateTime, nanosecond) &&
			offsetof(DateTime, day_of_week) == offsetof(SDL_DateTime, day_of_week) &&
			offsetof(DateTime, utc_offset_s) == offsetof(SDL_DateTime, utc_offset));
	bool is_local_time = false;
	switch (date_time_kind)
	{
		case DateTimeKind::local:
			is_local_time = true;
			break;
		case DateTimeKind::utc:
			break;
		default:
			BSTONE_THROW_STATIC_SOURCE("Unknown date-time kind.");
	}
	SDL_DateTime sdl_date_time;
	if (!SDL_TimeToDateTime(time_ns, &sdl_date_time, is_local_time))
	{
		sdl::fail("SDL_TimeToDateTime");
	}
	return DateTime{
		.year = sdl_date_time.year,
		.month = sdl_date_time.month,
		.day = sdl_date_time.day,
		.hour = sdl_date_time.hour,
		.minute = sdl_date_time.minute,
		.second = sdl_date_time.second,
		.nanosecond = sdl_date_time.nanosecond,
		.day_of_week = sdl_date_time.day_of_week,
		.utc_offset_s = sdl_date_time.utc_offset,
	};
}

} // namespace bstone::sys
