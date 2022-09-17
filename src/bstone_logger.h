/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// A logger.
//


#ifndef BSTONE_LOGGER_INCLUDED
#define BSTONE_LOGGER_INCLUDED


#include <memory>
#include <string>


namespace bstone
{


enum class LoggerMessageKind
{
	information,
	warning,
	error,
	critical_error,
}; // LoggerMessageKind


//
// Logger interface.
//
// Writes messages to standard output, file and
// shows message box on critical error.
//
class Logger
{
public:
	Logger() noexcept = default;

	virtual ~Logger() = default;


	// Writes a message of the specified kind.
	virtual void write(
		LoggerMessageKind message_kind,
		const std::string& message) noexcept = 0;

	// Write a new line.
	virtual void write() noexcept = 0;

	// Writes an informational message.
	virtual void write(
		const std::string& message) noexcept = 0;

	// Writes a warning message.
	virtual void write_warning(
		const std::string& message) noexcept = 0;

	// Writes an error message.
	virtual void write_error(
		const std::string& message) noexcept = 0;

	// Similar to error but with message box.
	virtual void write_critical(
		const std::string& message) noexcept = 0;
}; // Logger

using LoggerPtr = Logger*;
using LoggerUPtr = std::unique_ptr<Logger>;


struct LoggerFactory
{
	LoggerUPtr create();
}; // LoggerFactory


extern LoggerPtr logger_;

void log_exception() noexcept;


} // bstone


#endif // !BSTONE_LOGGER_INCLUDED
