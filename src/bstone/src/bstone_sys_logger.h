/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Logger

#ifndef BSTONE_SYS_LOGGER_INCLUDED
#define BSTONE_SYS_LOGGER_INCLUDED

#include "bstone_format.h"
#include <format>
#include <string>

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

	virtual void log(LogLevel level, const char* message) = 0;

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_information(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LogLevel::information, tls_string_.c_str());
	}

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_warning(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LogLevel::warning, tls_string_.c_str());
	}

	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_error(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LogLevel::error, tls_string_.c_str());
	}

private:
	static thread_local std::string tls_string_;
	static thread_local StdStringFormatIterator tls_format_iterator_;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_LOGGER_INCLUDED
