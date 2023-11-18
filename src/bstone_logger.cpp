/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


//
// Logging facility
//


#include "bstone_logger.h"

#include <iostream>
#include <mutex>

#include "bstone_exception.h"
#include "bstone_file_stream.h"
#include "bstone_sys_message_box.h"
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
		LoggerMessageType message_type,
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
	LoggerMessageType message_type,
	const std::string& message) noexcept
try
{
	MutexLock mutex_lock{mutex_};

	auto is_warning_or_error = false;
	auto is_critical = false;

	message_.clear();

	switch (message_type)
	{
		case LoggerMessageType::information:
			break;

		case LoggerMessageType::warning:
			is_warning_or_error = true;
			message_ += "[WARNING] ";
			break;

		case LoggerMessageType::error:
			is_warning_or_error = true;
			message_ += "[ERROR] ";
			break;

		case LoggerMessageType::critical_error:
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
		const auto new_line = '\n';
		file_stream_.write_exact(message_.c_str(), static_cast<std::intptr_t>(message.size()));
		file_stream_.write_exact(&new_line, 1);
		file_stream_.flush();
	}

	if (is_critical)
	{
		bstone::sys::show_message_box(
			get_message_box_title().c_str(),
			message_.c_str(),
			bstone::sys::MessageBoxType::error);
	}
}
catch (...)
{
	std::cerr << "Write failed." << std::endl;
	std::cerr << message.c_str() << std::endl;
}

void DefaultLogger::write() noexcept
{
	write(LoggerMessageType::information, "");
}

void DefaultLogger::write(
	const std::string& message) noexcept
{
	write(LoggerMessageType::information, message);
}

void DefaultLogger::write_warning(
	const std::string& message) noexcept
{
	write(LoggerMessageType::warning, message);
}

void DefaultLogger::write_error(
	const std::string& message) noexcept
{
	write(LoggerMessageType::error, message);
}

void DefaultLogger::write_critical(
	const std::string& message) noexcept
{
	write(LoggerMessageType::critical_error, message);
}

void DefaultLogger::initialize()
{
	{
		MutexLock mutex_lock{mutex_};

		const auto& profile_dir = get_profile_dir();
		file_name_ = profile_dir + "bstone_log.txt";

		try
		{
			file_stream_.open(
				file_name_.c_str(),
				FileOpenFlags::create | FileOpenFlags::truncate | FileOpenFlags::write);
		}
		catch (...)
		{
			std::cerr << "[ERROR] Failed to open a log file." << std::endl;
			std::cerr << "[ERROR] File: \"" << file_name_ << "\"." << std::endl;
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
