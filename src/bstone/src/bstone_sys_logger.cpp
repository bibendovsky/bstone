/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger.

#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

namespace {

constexpr const char* const empty_string = "";

} // namespace

Logger::Logger() = default;

Logger::~Logger() = default;

void Logger::log_information() noexcept
{
	log_information(empty_string);
}

void Logger::log_information(const char* message) noexcept
{
	do_log(LogLevel::information, message);
}

void Logger::log_warning() noexcept
{
	log_warning(empty_string);
}

void Logger::log_warning(const char* message) noexcept
{
	do_log(LogLevel::warning, message);
}

void Logger::log_error() noexcept
{
	log_error(empty_string);
}

void Logger::log_error(const char* message) noexcept
{
	do_log(LogLevel::error, message);
}

} // namespace sys
} // namespace bstone
