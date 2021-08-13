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

#include <algorithm>
#include <string>
#include <vector>

#include "bstone_index_type.h"


namespace bstone
{


namespace
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

static constexpr Index get_c_string_size(
	const char* string) noexcept
{
	assert(string);

	auto size = Index{};

	while (string[size] != '\0')
	{
		size += 1;
	}

	return size;
}


} // namespace


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
	assert(message);

	const auto has_contex = (context != nullptr);
	const auto context_size = (has_contex ? get_c_string_size(context) : 0);

	const auto has_message = (message != nullptr);
	const auto message_size = (has_message ? get_c_string_size(message) : 0);

	if (!has_contex && !has_message)
	{
		return;
	}

	constexpr auto left_prefix = "[";
	constexpr auto left_prefix_size = get_c_string_size(left_prefix);

	constexpr auto right_prefix = "] ";
	constexpr auto right_prefix_size = get_c_string_size(right_prefix);

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
	const auto what_size = get_c_string_size(rhs_what);

	what_.reset(new (std::nothrow) char[what_size + 1]);

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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

std::string get_nested_message()
{
	using Messages = std::vector<std::string>;


	static void (*extract_message)(
		Messages& messages) = [](
			Messages& messages)
	{
		try
		{
			std::rethrow_exception(std::current_exception());
		}
		catch (const std::exception& ex)
		{
			messages.emplace_back(ex.what());

			try
			{
				std::rethrow_if_nested(ex);
			}
			catch (...)
			{
				extract_message(messages);
			}
		}
		catch (...)
		{
			messages.emplace_back("[BSTONE_EXCEPTION] Generic error.");
		}
	};

	auto messages = Messages{};
	messages.reserve(16);

	extract_message(messages);

	auto message_size = std::string::size_type{};

	for (const auto& message : messages)
	{
		message_size += message.size() + 1;
	}

	std::reverse(messages.begin(), messages.end());

	auto result = std::string{};
	result.reserve(message_size);

	for (const auto& message : messages)
	{
		if (!result.empty())
		{
			result += '\n';
		}

		result += message;
	}

	return result;
}

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone
