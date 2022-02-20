/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
#include <mutex>

#include "SDL_messagebox.h"

#include "bstone_file_stream.h"
#include "bstone_version.h"


const std::string& get_profile_dir();

const std::string& get_message_box_title();


namespace bstone
{


LoggerPtr logger_ = nullptr;


// ==========================================================================
// DefaultLogger
//

class DefaultLogger final :
	public Logger
{
public:
	DefaultLogger();


	void write(
		LoggerMessageKind message_kind,
		const std::string& message) noexcept override;

	void write() noexcept override;

	void write(
		const std::string& message) noexcept override;

	void write_warning(
		const std::string& message) noexcept override;

	void write_error(
		const std::string& message) noexcept override;

	void write_critical(
		const std::string& message) noexcept override;


private:
	using Mutex = std::mutex;
	using MutexLock = std::lock_guard<Mutex>;


	Mutex mutex_{};
	std::string file_name_{};
	std::string message_{};
	FileStream file_stream_{};


	void initialize();
}; // DefaultLogger


DefaultLogger::DefaultLogger()
{
	initialize();
}

void DefaultLogger::write(
	LoggerMessageKind message_kind,
	const std::string& message) noexcept
try
{
	MutexLock mutex_lock{mutex_};

	auto is_warning_or_error = false;
	auto is_critical = false;

	message_.clear();

	switch (message_kind)
	{
		case LoggerMessageKind::information:
			break;

		case LoggerMessageKind::warning:
			is_warning_or_error = true;
			message_ += "[WARNING] ";
			break;

		case LoggerMessageKind::error:
			is_warning_or_error = true;
			message_ += "[ERROR] ";
			break;

		case LoggerMessageKind::critical_error:
			is_warning_or_error = true;
			is_critical = true;
			message_ += "[CRITICAL] ";
			break;

		default:
			return;
	}

	message_ += message;

	(is_warning_or_error ? std::cerr : std::cout) << message_ << std::endl;

	if (file_stream_.is_open())
	{
		static_cast<void>(file_stream_.write_string(message_));
		static_cast<void>(file_stream_.write_octet('\n'));
		file_stream_.flush();
	}

	if (is_critical)
	{
		static_cast<void>(::SDL_ShowSimpleMessageBox(
			::SDL_MESSAGEBOX_ERROR,
			get_message_box_title().c_str(),
			message_.c_str(),
			nullptr)
		);
	}
}
catch (...)
{
	std::cerr << "Write failed." << std::endl;
	std::cerr << message.c_str() << std::endl;
}

void DefaultLogger::write() noexcept
{
	write(LoggerMessageKind::information, "");
}

void DefaultLogger::write(
	const std::string& message) noexcept
{
	write(LoggerMessageKind::information, message);
}

void DefaultLogger::write_warning(
	const std::string& message) noexcept
{
	write(LoggerMessageKind::warning, message);
}

void DefaultLogger::write_error(
	const std::string& message) noexcept
{
	write(LoggerMessageKind::error, message);
}

void DefaultLogger::write_critical(
	const std::string& message) noexcept
{
	write(LoggerMessageKind::critical_error, message);
}

void DefaultLogger::initialize()
{
	{
		MutexLock mutex_lock{mutex_};

		const auto& profile_dir = get_profile_dir();
		file_name_ = profile_dir + "bstone_log.txt";

		{
			const auto is_file_open = file_stream_.open(file_name_, StreamOpenMode::write);

			if (!is_file_open)
			{
				std::cerr << "[ERROR] Failed to open a log file." << std::endl;
				std::cerr << "[ERROR] File: \"" << file_name_ << "\"." << std::endl;
			}
		}

		message_.reserve(1024);
	}

	write("BStone v" + bstone::Version::get_string());
	write("==========");
	write();
}

//
// DefaultLogger
// ==========================================================================


// ==========================================================================
// LoggerFactory
//

LoggerUPtr LoggerFactory::create()
{
	return std::make_unique<DefaultLogger>();
}

//
// LoggerFactory
// ==========================================================================

namespace
{

void log_exception_internal(std::string& message_buffer)
{
	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (const std::exception& ex)
	{
		const auto nex = dynamic_cast<const std::nested_exception*>(&ex);

		if (nex && nex->nested_ptr())
		{
			try
			{
				nex->rethrow_nested();
			}
			catch (...)
			{
				log_exception_internal(message_buffer);
			}
		}

		if (!message_buffer.empty())
		{
			message_buffer += '\n';
		}

		message_buffer += ex.what();
	}
	catch (...)
	{
		if (!message_buffer.empty())
		{
			message_buffer += '\n';
		}

		message_buffer += "Non-standard exception.";
	}
}

} // namespace

void log_exception() noexcept
try
{
	auto message_buffer = std::string{};
	message_buffer.reserve(1'024);
	log_exception_internal(message_buffer);
}
catch (const std::exception& ex)
{
	logger_->write_error(__func__);
	logger_->write_error(ex.what());
}
catch (...)
{
	logger_->write_error(__func__);
	logger_->write_error("Non-standard exception.");
}

} // bstone
