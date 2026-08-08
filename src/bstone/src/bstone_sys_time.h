/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Date and time management

#ifndef BSTONE_SYS_DATE_TIME_INCLUDED
#define BSTONE_SYS_DATE_TIME_INCLUDED

namespace bstone::sys {

// A number of nanoseconds.
using TimeNs = long long;

enum DateTimeKind
{
	none = 0,
	local,
	utc,
};

struct DateTime
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int nanosecond;
	int day_of_week; // Day of the week [0-6] (0 - Sunday).
	int utc_offset_s; // UTC offset in seconds.
};

// The number of non-leap nanoseconds that have elapsed since 00:00:00 UTC on 1 January 1970.
// The system clock can be stepped at any moment, so do not measure durations with it.
TimeNs get_current_time_ns();

// The number of nanoseconds that have elapsed since an unspecified moment in the past.
// Never goes backwards, hence the source to measure durations with.
TimeNs get_elapsed_time_ns();

DateTime time_ns_to_date_time(TimeNs time_ns, DateTimeKind date_time_kind);

} // namespace bstone::sys

#endif // BSTONE_SYS_DATE_TIME_INCLUDED
