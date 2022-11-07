/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_cl_args.h"
#include "bstone_exception.h"
#include "bstone_string_helper.h"

namespace bstone {

namespace {

class ClArgsException : public Exception
{
public:
	explicit ClArgsException(const char* message) noexcept
		:
		Exception{"BSTONE_CL_ARGS", message}
	{}
};

} // namespace

ClArgs::ClArgs() = default;

StringView ClArgs::operator[](int index) const
{
	return get_argument(index);
}

void ClArgs::initialize(int argc, char* const* argv)
{
	args_.clear();
	args_.reserve(argc);

	for (auto i = 0; i < argc; ++i)
	{
		args_.emplace_back(argv[i]);
	}
}

bool ClArgs::has_option(StringView option_name) const
{
	return find_option(option_name) >= 0;
}

bool ClArgs::has_option(const char* option_name) const
{
	return has_option(StringView{option_name});
}

Int ClArgs::find_option(StringView option_name) const
{
	if (option_name.is_empty())
	{
		return -1;
	}

	constexpr auto dash_prefix = StringView{"--"};

	auto index = Int{};

	for (const auto& arg : args_)
	{
		if (arg.starts_with(dash_prefix) && arg.get_subview(dash_prefix.get_size()) == option_name)
		{
			return index;
		}

		index += 1;
	}

	return -1;
}

Int ClArgs::get_count() const noexcept
{
	return static_cast<Int>(args_.size());
}

StringView ClArgs::get_argument(Int index) const
{
	if (index < 0 || index >= get_count())
	{
		return StringView{};
	}

	return args_[index];
}

StringView ClArgs::get_option_value(StringView option_name) const
{
	auto option_index = find_option(option_name);

	if (option_index >= 0)
	{
		option_index += 1;
	}

	return get_argument(option_index);
}

StringView ClArgs::get_option_value(const char* option_name) const
{
	return get_option_value(StringView{option_name});
}

void ClArgs::get_option_values(StringView option_name, StringView& value1, StringView& value2) const
{
	value1 = StringView{};
	value2 = StringView{};
	const auto option_index = find_option(option_name);

	if (option_index < 0)
	{
		return;
	}

	value1 = get_argument(option_index + 1);
	value2 = get_argument(option_index + 2);
}

[[noreturn]] void ClArgs::fail(const char* message)
{
	throw ClArgsException{message};
}

[[noreturn]] void ClArgs::fail_nested(const char* message)
{
	std::throw_with_nested(ClArgsException{message});
}

} // bstone
