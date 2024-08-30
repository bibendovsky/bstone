/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <cassert>

#include <exception>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

#include "bstone_tester.h"

namespace bstone {

namespace {

class EmptyCheckException : public std::exception
{
public:
	EmptyCheckException() = default;
	~EmptyCheckException() override = default;

	const char* what() const noexcept override
	{
		return "EmptyCheckException";
	}
};

} // namespace

// ==========================================================================

void Tester::register_test(const char* description, TesterFunc func)
{
	assert(description != nullptr);
	assert(func != nullptr);

	auto& registry = get_registry();
	registry.emplace_back();
	auto& registry_item = registry.back();
	registry_item.description = description;
	registry_item.tester_func = func;
}

[[noreturn]] void Tester::fail(const char* message)
{
	std::throw_with_nested(std::runtime_error{message});
}

void Tester::check(bool value, const char* message)
{
	if (!value)
	{
		fail(message);
	}
}

void Tester::check(bool value)
{
	if (!value)
	{
		std::throw_with_nested(EmptyCheckException{});
	}
}

bool Tester::test()
{
	const auto& registry = get_registry();
	auto max_description_size = std::size_t{};

	for (const auto& registry_item : registry)
	{
		const auto description_size = std::string::traits_type::length(registry_item.description);
		max_description_size = std::max(description_size, max_description_size);
	}

	max_description_size += 3;

	std::cout << "Test count: " << registry.size() << std::endl;
	std::cout << std::endl;
	static constexpr auto passed_string = "PASSED";
	static constexpr auto failed_string = "FAILED";

	auto passed_count = 0;
	auto failed_count = 0;

	for (const auto& registry_item : registry)
	{
		std::cout << std::setw(max_description_size) << std::left;
		std::cout << registry_item.description;

		try
		{
			registry_item.tester_func();
			passed_count += 1;
			std::cout << passed_string << std::endl;
		}
		catch (...)
		{
			failed_count += 1;
			std::cout << failed_string << std::endl;
			log_exception();
		}
	}

	std::cout << std::endl;
	std::cout << "Passed count: " << passed_count << std::endl;
	std::cout << "Failed count: " << failed_count << std::endl;
	std::cout << std::endl;
	std::cout << (failed_count == 0 ? passed_string : failed_string) << std::endl;

	return failed_count == 0;
}

Tester::Registry& Tester::get_registry()
{
	static auto registry = Registry{};
	return registry;
}

void Tester::log_exception()
{
	const auto handle_catch = [](const std::exception& exception, bool is_empty_check)
	{
		const auto nested_exception = dynamic_cast<const std::nested_exception*>(&exception);

		if (nested_exception != nullptr && nested_exception->nested_ptr())
		{
			try
			{
				nested_exception->rethrow_nested();
			}
			catch (...)
			{
				log_exception();
			}
		}

		if (!is_empty_check)
		{
			assert(exception.what() != nullptr);
			std::cerr << "[ERROR] " << exception.what() << std::endl;
		}
	};

	try
	{
		std::rethrow_exception(std::current_exception());
	}
	catch (const EmptyCheckException& exception)
	{
		handle_catch(exception, true);
	}
	catch (const std::exception& exception)
	{
		handle_catch(exception, false);
	}
	catch (...)
	{
		std::cerr << "[ERROR] Generic failure." << std::endl;
	}
}

} // namespace bstone
