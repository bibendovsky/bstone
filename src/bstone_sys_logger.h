/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger.

#ifndef BSTONE_SYS_LOGGER_INCLUDED
#define BSTONE_SYS_LOGGER_INCLUDED

namespace bstone {
namespace sys {

enum class LogLevel
{
	none,
	information,
};

// ==========================================================================

class Logger
{
public:
	Logger();
	virtual ~Logger();

	void log_information() noexcept;
	void log_information(const char* message) noexcept;

private:
	virtual void do_log(LogLevel level, const char* message) noexcept = 0;
};

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_LOGGER_INCLUDED
