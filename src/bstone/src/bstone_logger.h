/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Logger

#ifndef BSTONE_LOGGER_INCLUDED
#define BSTONE_LOGGER_INCLUDED

#include "bstone_format.h"
#include <format>
#include <memory>
#include <string>
#include <string_view>

namespace bstone {

enum class LoggerMessageType
{
	information,
	warning,
	error,
};

// ==========================================================================

enum class LoggerFlushPolicy
{
	none,
	every_message,
};

// ==========================================================================

struct LoggerLimits
{
	static constexpr auto max_message_length = 1 << 24;

	static_assert(max_message_length > 0 && max_message_length <= 1 << 24, "Max message length out of range.");
};

// ==========================================================================

// Writes messages to standard output and file.
class Logger
{
public:
	Logger() = default;
	virtual ~Logger() = default;

	// Writes a message of the specified type.
	virtual void log(LoggerMessageType message_type, std::string_view message_sv) = 0;

	// Write an empty informational message.
	void log_information();

	// Writes an informational message.
	void log_information(std::string_view message_sv);

	// Writes a warning message.
	void log_warning(std::string_view message_sv);

	// Writes an error message.
	void log_error(std::string_view message_sv);

	// Writes an error message for a specified exception.
	void log_exception(std::exception_ptr exception_ptr);

	// Writes an error message for the current exception.
	void log_current_exception();

	// Flushes all remaining messages.
	// (blocks the calling thread)
	virtual void flush() = 0;

	// Writes a formatted informational message.
	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_information(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LoggerMessageType::information, tls_string_);
	}

	// Writes a formatted warning message.
	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_warning(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LoggerMessageType::warning, tls_string_);
	}

	// Writes a formatted error message.
	template<typename... TArgs>
	requires (sizeof...(TArgs) > 0)
	void log_error(std::format_string<TArgs...> format_string, TArgs&&... args)
	{
		tls_string_.clear();
		std::vformat_to(tls_format_iterator_, format_string.get(), std::make_format_args(args...));
		log(LoggerMessageType::error, tls_string_);
	}

private:
	static thread_local std::string tls_string_;
	static thread_local StdStringFormatIterator tls_format_iterator_;

	void log_exception_internal(std::exception_ptr exception_ptr, std::string& message_buffer);
};

// ==========================================================================

struct LoggerOpenParam
{
	bool is_synchronous{};
	LoggerFlushPolicy flush_policy{};
	const char* file_path{};
};

using LoggerUPtr = std::unique_ptr<Logger>;

LoggerUPtr make_logger(const LoggerOpenParam& param);

} // namespace bstone

#endif // BSTONE_LOGGER_INCLUDED
