/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


#ifndef BSTONE_TIME_INCLUDED
#define BSTONE_TIME_INCLUDED


#include <string>


namespace bstone
{


struct DateTime
{
	int year{};
	int month{}; // [1..12]
	int day{}; // [1..31]

	int hours{};
	int minutes{};
	int seconds{};
	int milliseconds{};
}; // DateTime

enum class DateTimeStringFormat
{
	none,
	screenshot_file_name,
}; // DateTimeStringFormat


DateTime make_local_date_time();

std::string make_local_date_time_string(
	const DateTime& date_time,
	DateTimeStringFormat format);


} // bstone


#endif // !BSTONE_TIME_INCLUDED
