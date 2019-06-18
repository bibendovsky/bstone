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


#include "bstone_log.h"
#include <iostream>
#include "SDL_messagebox.h"
#include "bstone_version.h"


const std::string& get_profile_dir();


namespace bstone
{


bool Log::is_initialized_;
FileStream Log::fstream_;
LogMessageType Log::message_type_;
std::string Log::message_;


void Log::initialize()
{
	if (is_initialized_)
	{
		return;
	}

	const auto& profile_dir = ::get_profile_dir();
	const auto& log_path = profile_dir + "bstone_log.txt";

	is_initialized_ = true;
	fstream_.open(log_path, StreamOpenMode::write);
	message_.reserve(1024);

	write("BStone v" + bstone::Version::get_string());
	write("==========");
	write();
}

void Log::write()
{
	write(std::string{});
}

void Log::write(
	const std::string& message)
{
	message_type_ = LogMessageType::information;
	write_internal(message);
}

void Log::write_warning(
	const std::string& message)
{
	message_type_ = LogMessageType::warning;
	write_internal(message);
}

void Log::write_error(
	const std::string& message)
{
	message_type_ = LogMessageType::error;
	write_internal(message);
}

void Log::write_critical(
	const std::string& message)
{
	message_type_ = LogMessageType::critical_error;
	write_internal(message);
}

void Log::write_internal(
	const std::string& message)
{
	auto is_critical = false;

	switch (message_type_)
	{
	case LogMessageType::information:
		message_.clear();
		break;

	case LogMessageType::warning:
		message_ = "WARNING: ";
		break;

	case LogMessageType::error:
		message_ = "ERROR: ";
		break;

	case LogMessageType::critical_error:
		is_critical = true;
		message_ = "CRITICAL: ";
		break;

	default:
		throw std::runtime_error("Invalid message type.");
	}

	message_ += message;

	std::cout << message_ << std::endl;

	fstream_.write_string(message_);
	fstream_.write_octet('\n');

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


} // bstone
