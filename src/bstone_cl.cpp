/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Command-line.

#include <cassert>
#include "bstone_cl.h"
#include "bstone_exception.h"

namespace bstone {

namespace {

class ClException : public Exception
{
public:
	explicit ClException(const char* message) noexcept
		:
		Exception{"BSTONE_CL", message}
	{}
};

constexpr auto cl_option_prefix = StringView{"--"};

} // namespace

Cl::Cl() = default;

StringView Cl::operator[](int index) const
{
	return get_argument(index);
}

void Cl::initialize(int argc, char* const* argv)
try
{
	args_.clear();
	args_.reserve(argc);

	options_.clear();
	options_.reserve(argc);

	auto arg_index = 1;

	// Add parameters.
	//
	auto param_count = Int{};

	while (arg_index < argc)
	{
		const auto arg_sv = StringView{argv[arg_index]};

		if (arg_sv.starts_with(cl_option_prefix))
		{
			if (arg_sv.get_size() == cl_option_prefix.get_size())
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
		params_ = ClArgs{args_.data(), param_count};
	}

	// Then add options.
	//
	while (arg_index < argc)
	{
		const auto name_sv = StringView{argv[arg_index]};
		assert(name_sv.starts_with(cl_option_prefix));
		args_.emplace_back(name_sv);
		options_.emplace_back();
		auto& option = options_.back();
		option.name = name_sv.get_subview(cl_option_prefix.get_size());
		assert(!option.name.is_empty());

		arg_index += 1;
		const auto option_args_index = static_cast<Int>(args_.size());
		auto option_arg_count = Int{};

		while (arg_index < argc)
		{
			const auto arg_sv = StringView{argv[arg_index]};

			if (arg_sv.starts_with(cl_option_prefix))
			{
				if (arg_sv.get_size() == cl_option_prefix.get_size())
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
			option.args = ClArgs{&args_[option_args_index], option_arg_count};
		}
	}
}
catch (...)
{
	fail_nested(__func__);
}

ClOptions Cl::get_options() const noexcept
{
	return bstone::make_span(options_.data(), static_cast<Int>(options_.size()));
}

bool Cl::has_option(StringView option_name) const
{
	return !find_option(option_name).name.is_empty();
}

#if FIXMENOW
bool Cl::has_option(const char* option_name) const
{
	return has_option(StringView{option_name});
}
#endif

ClOption Cl::find_option(StringView option_name) const
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

Int Cl::get_count() const noexcept
{
	return static_cast<Int>(args_.size());
}

StringView Cl::get_argument(Int index) const
{
	if (index < 0 || index >= get_count())
	{
		return StringView{};
	}

	return args_[index];
}

StringView Cl::get_option_value(StringView option_name) const
{
	const auto option = find_option(option_name);

	if (option.name.is_empty() || option.args.is_empty())
	{
		return StringView{};
	}

	return option.args.get_front();
}

#if FIXMENOW
StringView Cl::get_option_value(const char* option_name) const
{
	return get_option_value(StringView{option_name});
}
#endif

void Cl::get_option_values(StringView option_name, StringView& value1, StringView& value2) const
{
	value1 = StringView{};
	value2 = StringView{};

	const auto option = find_option(option_name);

	if (option.name.is_empty())
	{
		return;
	}

	const auto arg_count = option.args.get_size();

	if (arg_count >= 1)
	{
		value1 = option.args[0];
	}

	if (arg_count >= 2)
	{
		value2 = option.args[1];
	}
}

[[noreturn]] void Cl::fail(const char* message)
{
	throw ClException{message};
}

[[noreturn]] void Cl::fail_nested(const char* message)
{
	std::throw_with_nested(ClException{message});
}

} // bstone
