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


#ifndef BSTONE_LOG_INCLUDED
#define BSTONE_LOG_INCLUDED


#include <string>
#include "bstone_file_stream.h"


namespace bstone
{


enum class LogMessageType
{
	none,
	version,
	information,
	warning,
	error,
	critical_error,
}; // LogMessageType


//
// Writes messages to standard output, file and
// shows message box on critical error.
//
class Log
{
public:
	static void initialize();


	// Writes the game's version to standart output and shows a message box.
	static void write_version();


	// Write a new line.
	static void write();

	// Writes an informational message.
	static void write(
		const std::string& message);

	// Writes a warning message.
	static void write_warning(
		const std::string& message);

	// Writes an error message.
	static void write_error(
		const std::string& message);

	// Similar to error but with message box.
	static void write_critical(
		const std::string& message);


private:
	static void write_internal(
		const std::string& message);


	static bool is_initialized_;
	static FileStream fstream_;
	static LogMessageType message_type_;
	static std::string message_;
};


} // bstone


#endif // BSTONE_LOG_INCLUDED
