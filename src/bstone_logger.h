/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A logger.

#ifndef BSTONE_LOGGER_INCLUDED
#define BSTONE_LOGGER_INCLUDED

#include <cstdint>

#include <memory>
#include <string>

#include "bstone_string_view.h"

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
	Logger() noexcept = default;
	virtual ~Logger() = default;

	// Writes a message of the specified type.
	void write(LoggerMessageType message_type, const char* message) noexcept;

	// Writes a message of the specified type.
	void write(LoggerMessageType message_type, const std::string& message) noexcept;

	// Write a new line.
	void write() noexcept;

	// Writes an informational message.
	void write(const char* message) noexcept;

	// Writes an informational message.
	void write(const std::string& message) noexcept;

	// Writes a warning message.
	void write_warning(const char* message) noexcept;

	// Writes a warning message.
	void write_warning(const std::string& message) noexcept;

	// Writes an error message.
	void write_error(const char* message) noexcept;

	// Writes an error message.
	void write_error(const std::string& message) noexcept;

	// Writes an error message for current exception.
	void write_exception() noexcept;

private:
	virtual void do_write(LoggerMessageType message_type, StringView message_sv) noexcept = 0;

private:
	static StringView to_string_view(const std::string& string) noexcept;
	void write_exception_internal(std::string& message_buffer);
};

// ==========================================================================

struct LoggerOpenParam
{
	bool is_synchronous;
	LoggerFlushPolicy flush_policy;
	const char* file_path{};
};

using LoggerPtr = Logger*;
using LoggerUPtr = std::unique_ptr<Logger>;

LoggerUPtr make_logger(const LoggerOpenParam& param);

// ==========================================================================

extern LoggerPtr logger_;

} // namespace bstone

#endif // BSTONE_LOGGER_INCLUDED
