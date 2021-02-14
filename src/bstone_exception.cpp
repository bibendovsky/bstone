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
// Base exception.
//


#include "bstone_exception.h"


namespace bstone
{


Exception::Exception(
	const char* const message)
	:
	std::runtime_error{message}
{
}

Exception::Exception(
	const std::string& message)
	:
	std::runtime_error{message}
{
}

std::string Exception::get_nested_message(
	const std::exception& exception)
{
	constexpr auto reserve_size = std::string::size_type{1024};

	auto message = std::string{};
	message.reserve(reserve_size);

	get_nested_message(exception, message);

	return message;
}

void Exception::get_nested_message(
	const std::exception& exception,
	std::string& message)
{
	if (!message.empty())
	{
		message += '\n';
	}

	message += exception.what();

	try
	{
		std::rethrow_if_nested(exception);
	}
	catch (const std::exception& nex)
	{
		get_nested_message(nex, message);
	}
}


} // bstone
