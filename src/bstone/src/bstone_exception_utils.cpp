/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Exception utils.

#include <cassert>
#include <cstdint>
#include <string>

#include "bstone_char_traits.h"
#include "bstone_exception.h"
#include "bstone_exception_utils.h"

namespace bstone {

namespace {

void extract_exception_messages(ExceptionMessages& messages)
{
	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (const SourceException& exception)
	{
		{
			const auto& source_location = exception.get_source_location();
			const auto file_name = source_location.get_file_name();
			const auto file_name_size = char_traits::get_size(file_name);

			const auto source_line = source_location.get_line();

			const auto function_name = source_location.get_function_name();
			const auto function_name_size = char_traits::get_size(function_name);

			const auto what = exception.what();
			const auto what_size = char_traits::get_size(what);

			auto error_message = std::string{};
			error_message.reserve(file_name_size + function_name_size + what_size + 64);

			if (file_name_size > 0 || function_name_size > 0)
			{
				error_message += '(';

				if (file_name_size > 0)
				{
					error_message.append(file_name, static_cast<std::size_t>(file_name_size));
				}

				if (file_name_size > 0)
				{
					error_message += ':';
				}

				error_message += std::to_string(source_line);

				if (function_name_size > 0)
				{
					error_message += ':';
					error_message.append(function_name, static_cast<std::size_t>(function_name_size));
				}

				error_message += ')';
			}

			if (what_size > 0)
			{
				if (!error_message.empty())
				{
					error_message += ' ';
				}

				error_message.append(what, static_cast<std::size_t>(what_size));
			}

			messages.emplace_back(error_message);
		}

		try
		{
			std::rethrow_if_nested(exception);
		}
		catch (...)
		{
			extract_exception_messages(messages);
		}
	}
	catch (const std::exception& exception)
	{
		messages.emplace_back(exception.what());

		try
		{
			std::rethrow_if_nested(exception);
		}
		catch (...)
		{
			extract_exception_messages(messages);
		}
	}
	catch (...)
	{
		messages.emplace_back("Generic failure.");
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
