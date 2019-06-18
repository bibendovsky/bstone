/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2019 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// Logging facility
//


#include "bstone_logger.h"
#include <iostream>
#include "SDL_messagebox.h"
#include "bstone_file_stream.h"
#include "bstone_version.h"


const std::string& get_profile_dir();


namespace bstone
{


LoggerPtr logger_ = nullptr;


// ==========================================================================
// Logger
//

Logger::Logger()
{
}

Logger::~Logger()
{
}

//
// Logger
// ==========================================================================


// ==========================================================================
// DefaultLogger
//

class DefaultLogger :
	public Logger
{
public:
	DefaultLogger();

	DefaultLogger(
		const DefaultLogger& rhs) = delete;

	DefaultLogger(
		DefaultLogger&& rhs);

	~DefaultLogger() override;


	void write(
		const LoggerMessageKind message_kind,
		const std::string& message) override;

	void write() override;

	void write(
		const std::string& message) override;

	void write_warning(
		const std::string& message) override;

	void write_error(
		const std::string& message) override;

	void write_critical(
		const std::string& message) override;


private:
	bool is_file_stream_initialized_;
	std::string message_;
	FileStream file_stream_;


	void initialize();

	void uninitialize();
}; // DefaultLogger


DefaultLogger::DefaultLogger()
	:
	is_file_stream_initialized_{},
	message_{},
	file_stream_{}
{
	initialize();
}

DefaultLogger::DefaultLogger(
	DefaultLogger&& rhs)
	:
	is_file_stream_initialized_{std::move(rhs.is_file_stream_initialized_)},
	message_{std::move(rhs.message_)},
	file_stream_{std::move(rhs.file_stream_)}
{
}

DefaultLogger::~DefaultLogger()
{
	uninitialize();
}

void DefaultLogger::write(
	const LoggerMessageKind message_kind,
	const std::string& message)
{
	auto is_critical = false;

	switch (message_kind)
	{
	case LoggerMessageKind::information:
		message_.clear();
		break;

	case LoggerMessageKind::warning:
		message_ = "WARNING: ";
		break;

	case LoggerMessageKind::error:
		message_ = "ERROR: ";
		break;

	case LoggerMessageKind::critical_error:
		is_critical = true;
		message_ = "CRITICAL: ";
		break;

	default:
		throw std::runtime_error("Invalid message type.");
	}

	message_ += message;

	std::cout << message_ << std::endl;

	if (is_file_stream_initialized_)
	{
		static_cast<void>(file_stream_.write_string(message_));
		static_cast<void>(file_stream_.write_octet('\n'));
	}

	if (is_critical)
	{
		static_cast<void>(::SDL_ShowSimpleMessageBox(
			SDL_MESSAGEBOX_ERROR,
			"BStone",
			message_.c_str(),
			nullptr)
		);
	}
}

void DefaultLogger::write()
{
	static auto empty_message = std::string{""};

	write(LoggerMessageKind::information, empty_message);
}

void DefaultLogger::write(
	const std::string& message)
{
	write(LoggerMessageKind::information, message);
}

void DefaultLogger::write_warning(
	const std::string& message)
{
	write(LoggerMessageKind::warning, message);
}

void DefaultLogger::write_error(
	const std::string& message)
{
	write(LoggerMessageKind::error, message);
}

void DefaultLogger::write_critical(
	const std::string& message)
{
	write(LoggerMessageKind::critical_error, message);
}

void DefaultLogger::initialize()
{
	const auto& profile_dir = ::get_profile_dir();
	const auto& log_path = profile_dir + "bstone_log.txt";

	is_file_stream_initialized_ = file_stream_.open(log_path, StreamOpenMode::write);

	if (!is_file_stream_initialized_)
	{
		std::cout << "ERROR: Failed to open a log file." << std::endl;
		std::cout << "ERROR: File: \"" << log_path << "\"." << std::endl;
	}

	message_.reserve(1024);

	write("BStone v" + bstone::Version::get_string());
	write("==========");
	write();
}

void DefaultLogger::uninitialize()
{
	is_file_stream_initialized_ = false;
	message_.clear();
	file_stream_.close();
}

//
// DefaultLogger
// ==========================================================================


// ==========================================================================
// LoggerFactory
//

LoggerUPtr LoggerFactory::create()
{
	return LoggerUPtr{new DefaultLogger{}};
}

//
// LoggerFactory
// ==========================================================================


} // bstone
