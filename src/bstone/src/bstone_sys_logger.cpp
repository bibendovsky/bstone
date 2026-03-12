/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger

#include "bstone_sys_logger.h"

namespace bstone::sys {

thread_local std::string Logger::tls_string_(std::string::size_type{256}, '\0');
thread_local StdStringFormatIterator Logger::tls_format_iterator_{tls_string_};

// ======================================

void Logger::log_information()
{
	log_information("");
}

void Logger::log_information(const char* message)
{
	log(LogLevel::information, message);
}

void Logger::log_warning()
{
	log_warning("");
}

void Logger::log_warning(const char* message)
{
	log(LogLevel::warning, message);
}

void Logger::log_error()
{
	log_error("");
}

void Logger::log_error(const char* message)
{
	log(LogLevel::error, message);
}

} // namespace bstone::sys
