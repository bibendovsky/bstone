/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// Base exception.
//

#include <cassert>
#include <string>
#include <vector>
#include "bstone_exception.h"
#include "bstone_int.h"

namespace bstone {

namespace {

static constexpr Int get_c_string_size(const char* string) noexcept
{
	assert(string);

	auto size = Int{};

	while (string[size] != '\0')
	{
		size += 1;
	}

	return size;
}

} // namespace

// ==========================================================================

Exception::Exception(const char* context, const char* message) noexcept
{
	assert(message != nullptr);

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

Exception::Exception(const Exception& rhs) noexcept
{
	if (rhs.what_ == nullptr)
	{
		return;
	}

	const auto rhs_what = rhs.what_.get();
	const auto what_size = get_c_string_size(rhs_what);

	what_.reset(new (std::nothrow) char[what_size + 1]);

	if (what_ == nullptr)
	{
		return;
	}

	auto what = what_.get();
	what = std::uninitialized_copy_n(rhs_what, what_size, what);
	*what = '\0';
}

Exception::~Exception() = default;

const char* Exception::what() const noexcept
{
	return what_ != nullptr ? what_.get() : "[BSTONE_EXCEPTION] Generic failure.";
}

// ==========================================================================

StaticException::StaticException(const char* file_name, int line, const char* function_name) noexcept
	:
	StaticException{file_name, line, function_name, nullptr}
{}

StaticException::StaticException(
	const char* file_name,
	int line,
	const char* function_name,
	const char* message) noexcept
	:
	file_name_{file_name},
	line_{line},
	function_name_{function_name},
	message_{message}
{}

const char* StaticException::get_file_name() const noexcept
{
	return file_name_;
}

int StaticException::get_line() const noexcept
{
	return line_;
}

const char* StaticException::get_function_name() const noexcept
{
	return function_name_;
}

const char* StaticException::get_message() const noexcept
{
	return message_;
}

const char* StaticException::what() const noexcept
{
	return message_ != nullptr ? message_ : "Generic failure.";
}

// --------------------------------------------------------------------------

[[noreturn]] void static_fail(
	const char* file_name,
	int line,
	const char* function_name,
	const char* message)
{
	throw StaticException{file_name, line, function_name, message};
}

[[noreturn]] void static_fail_nested(
	const char* file_name,
	int line,
	const char* function_name)
{
	std::throw_with_nested(StaticException{file_name, line, function_name});
}

// ==========================================================================

namespace {

void extract_exception_messages(ExceptionMessages& messages)
{
	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (const StaticException& ex)
	{
		{
			auto message = std::string{};
			message.reserve(1024);
			message += ex.get_file_name();
			message += '(';
			message += std::to_string(ex.get_line());
			message += ") : ";

			if (ex.get_function_name() != nullptr)
			{
				message += ex.get_function_name();
				message += " : ";
			}

			messages.emplace_back(message);
		}

		try
		{
			std::rethrow_if_nested(ex);
		}
		catch (...)
		{
			extract_exception_messages(messages);
		}
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
			extract_exception_messages(messages);
		}
	}
	catch (...)
	{
		messages.emplace_back("[BSTONE_EXCEPTION] Generic failure.");
	}
}

} // namespace


ExceptionMessages extract_exception_messages()
{
	auto messages = ExceptionMessages{};
	extract_exception_messages(messages);

	return messages;
}

// ==========================================================================

std::string get_nested_message()
{
	auto messages = extract_exception_messages();
	auto message_size = std::string::size_type{};

	for (const auto& message : messages)
	{
		message_size += message.size() + 1;
	}

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

} // namespace bstone
