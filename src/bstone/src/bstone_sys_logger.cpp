/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger.

#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

Logger::Logger() = default;

Logger::~Logger() = default;

void Logger::log_information() noexcept
{
	log_information("");
}

void Logger::log_information(const char* message) noexcept
{
	do_log(LogLevel::information, message);
}

} // namespace sys
} // namespace bstone
