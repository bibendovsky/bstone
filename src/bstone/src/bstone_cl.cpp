/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Command-line.

#include "bstone_cl.h"
#include "bstone_assert.h"
#include "bstone_exception.h"

namespace bstone {

namespace {

constexpr auto cl_option_prefix = std::string_view{"--"};

} // namespace

Cl::Cl() = default;

std::string_view Cl::operator[](int index) const
{
	return get_argument(index);
}

void Cl::initialize(int argc, char* const* argv)
try {
	args_.clear();
	args_.reserve(argc);

	options_.clear();
	options_.reserve(argc);

	auto arg_index = 1;

	// Add parameters.
	//
	auto param_count = std::intptr_t{};

	while (arg_index < argc)
	{
		const auto arg_sv = std::string_view{argv[arg_index]};

		if (arg_sv.starts_with(cl_option_prefix))
		{
			if (arg_sv.size() == cl_option_prefix.size())
			{
				const auto next_arg_index = arg_index + 1;

				if (next_arg_index < argc)
				{
					param_count += 1;
					args_.emplace_back(argv[next_arg_index]);
				}

				arg_index += 2;
			}
			else
			{
				break;
			}
		}
		else
		{
			arg_index += 1;
			param_count += 1;
			args_.emplace_back(arg_sv);
		}
	}

	if (param_count > 0)
	{
		params_ = ClArgs{args_.data(), static_cast<std::size_t>(param_count)};
	}

	// Then add options.
	//
	while (arg_index < argc)
	{
		const auto name_sv = std::string_view{argv[arg_index]};
		BSTONE_ASSERT(name_sv.starts_with(cl_option_prefix));
		args_.emplace_back(name_sv);
		options_.emplace_back();
		auto& option = options_.back();
		option.name = name_sv.substr(cl_option_prefix.size());
		BSTONE_ASSERT(!option.name.empty());

		arg_index += 1;
		const auto option_args_index = static_cast<std::intptr_t>(args_.size());
		auto option_arg_count = std::intptr_t{};

		while (arg_index < argc)
		{
			const auto arg_sv = std::string_view{argv[arg_index]};

			if (arg_sv.starts_with(cl_option_prefix))
			{
				if (arg_sv.size() == cl_option_prefix.size())
				{
					const auto next_arg_index = arg_index + 1;

					if (next_arg_index < argc)
					{
						option_arg_count += 1;
						args_.emplace_back(argv[next_arg_index]);
					}

					arg_index += 2;
				}
				else
				{
					break;
				}
			}
			else
			{
				arg_index += 1;
				option_arg_count += 1;
				args_.emplace_back(arg_sv);
			}
		}

		if (option_arg_count > 0)
		{
			option.args = ClArgs{&args_[option_args_index], static_cast<std::size_t>(option_arg_count)};
		}
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

ClOptions Cl::get_options() const noexcept
{
	return std::span{options_.data(), options_.size()};
}

bool Cl::has_option(std::string_view option_name) const
{
	return !find_option(option_name).name.empty();
}

ClOption Cl::find_option(std::string_view option_name) const
{
	for (const auto& option : options_)
	{
		if (option.name == option_name)
		{
			return option;
		}
	}

	return ClOption{};
}

std::intptr_t Cl::get_count() const noexcept
{
	return static_cast<std::intptr_t>(args_.size());
}

std::string_view Cl::get_argument(std::intptr_t index) const
{
	if (index < 0 || index >= get_count())
	{
		return std::string_view{};
	}

	return args_[index];
}

std::string_view Cl::get_option_value(std::string_view option_name) const
{
	const auto option = find_option(option_name);

	if (option.name.empty() || option.args.empty())
	{
		return std::string_view{};
	}

	return option.args.front();
}

void Cl::get_option_values(std::string_view option_name, std::string_view& value1, std::string_view& value2) const
{
	value1 = std::string_view{};
	value2 = std::string_view{};

	const auto option = find_option(option_name);

	if (option.name.empty())
	{
		return;
	}

	const auto arg_count = option.args.size();

	if (arg_count >= 1)
	{
		value1 = option.args[0];
	}

	if (arg_count >= 2)
	{
		value2 = option.args[1];
	}
}

} // bstone
