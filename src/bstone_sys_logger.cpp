/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

void Logger::log_information() noexcept
{
	log_information("");
}

void Logger::log_information(const std::string& message) noexcept
{
	do_log(LogLevel::information, message);
}

} // namespace sys
} // namespace bstone
