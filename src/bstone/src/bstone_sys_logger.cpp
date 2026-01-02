/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger

#include "bstone_sys_logger.h"

namespace bstone::sys {

void Logger::log_information()
{
	log_information("");
}

void Logger::log_information(const char* message)
{
	do_log(LogLevel::information, message);
}

void Logger::log_warning()
{
	log_warning("");
}

void Logger::log_warning(const char* message)
{
	do_log(LogLevel::warning, message);
}

void Logger::log_error()
{
	log_error("");
}

void Logger::log_error(const char* message)
{
	do_log(LogLevel::error, message);
}

} // namespace bstone::sys
