/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger

#ifndef BSTONE_SYS_LOGGER_INCLUDED
#define BSTONE_SYS_LOGGER_INCLUDED

namespace bstone::sys {

enum class LogLevel
{
	none,
	information,
	warning,
	error,
};

// ======================================

class Logger
{
public:
	Logger() = default;
	virtual ~Logger() = default;

	void log_information();
	void log_information(const char* message);

	void log_warning();
	void log_warning(const char* message);

	void log_error();
	void log_error(const char* message);

private:
	virtual void do_log(LogLevel level, const char* message) = 0;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_LOGGER_INCLUDED
