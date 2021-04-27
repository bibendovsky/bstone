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

#include <cassert>

#include "bstone_index_type.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class Exception::Detail
{
public:
	static constexpr Index get_c_string_size(
		const char* c_string) noexcept
	{
		assert(c_string);

		auto size = Index{};

		while (c_string[size] != '\0')
		{
			size += 1;
		}

		return size;
	}
}; // Exception::Detail

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

Exception::Exception(
	const char* message) noexcept
	:
	Exception{nullptr, message}
{
}

Exception::Exception(
	const std::string& message) noexcept
	:
	Exception{nullptr, message.c_str()}
{
}

Exception::Exception(
	const char* context,
	const char* message) noexcept
{
	const auto context_size = (context ? Detail::get_c_string_size(context) : 0);
	const auto has_contex = (context_size > 0);

	const auto is_null_message = (message == nullptr);
	const auto message_size = (!is_null_message ? Detail::get_c_string_size(message) : 0);
	const auto has_message = (message_size > 0);

	if (!has_contex && !has_message && is_null_message)
	{
		return;
	}

	static constexpr auto left_prefix = "[";
	static constexpr auto left_prefix_size = Detail::get_c_string_size(left_prefix);

	static constexpr auto right_prefix = "] ";
	static constexpr auto right_prefix_size = Detail::get_c_string_size(right_prefix);

	const auto what_size =
		(
			has_contex ?
			left_prefix_size + context_size + right_prefix_size :
			0
		) +
		message_size +
		1
	;

	what_.reset(new (std::nothrow) char[what_size]);

	if (!what_)
	{
		return;
	}

	auto what = what_.get();

	if (has_contex)
	{
		what = std::uninitialized_copy_n(left_prefix, left_prefix_size, what);
		what = std::uninitialized_copy_n(context, context_size, what);
		what = std::uninitialized_copy_n(right_prefix, right_prefix_size, what);
	}

	if (has_message)
	{
		what = std::uninitialized_copy_n(message, message_size, what);
	}

	*what = '\0';
}

Exception::Exception(
	const Exception& rhs) noexcept
{
	if (!rhs.what_)
	{
		return;
	}

	const auto rhs_what = rhs.what_.get();
	const auto what_size = Detail::get_c_string_size(rhs_what);

	what_.reset(new (std::nothrow) char[what_size]);

	if (!what_)
	{
		return;
	}

	auto what = what_.get();
	what = std::uninitialized_copy_n(rhs_what, what_size, what);
	*what = '\0';
}

const char* Exception::what() const noexcept
{
	return what_ ? what_.get() : "[BSTONE_EXCEPTION] Generic failure.";
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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
