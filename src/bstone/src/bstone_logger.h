/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A logger.

#ifndef BSTONE_LOGGER_INCLUDED
#define BSTONE_LOGGER_INCLUDED

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
	Logger();
	virtual ~Logger();

	// Writes a message of the specified type.
	void log(LoggerMessageType message_type, std::string_view message_sv);

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
	void flush();

private:
	virtual void do_log(LoggerMessageType message_type, std::string_view message_sv) = 0;
	virtual void do_flush() = 0;

private:
	void log_exception_internal(std::exception_ptr exception_ptr, std::string& message_buffer);
};

// ==========================================================================

struct LoggerOpenParam
{
	bool is_synchronous{};
	LoggerFlushPolicy flush_policy{};
	const char* file_path{};
};

using LoggerPtr = Logger*;
using LoggerUPtr = std::unique_ptr<Logger>;

LoggerUPtr make_logger(const LoggerOpenParam& param);

} // namespace bstone

#endif // BSTONE_LOGGER_INCLUDED
