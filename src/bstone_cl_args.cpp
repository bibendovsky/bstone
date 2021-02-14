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


#include "bstone_cl_args.h"

#include "bstone_string_helper.h"


namespace bstone
{


ClArgs::ClArgs() :
	args_{},
	lc_args_{}
{
}

const std::string& ClArgs::operator[](
	const int index) const
{
	return get_argument(index);
}

void ClArgs::initialize(
	const int argc,
	char* const* argv)
{
	unintialize();

	args_.resize(argc);
	lc_args_.resize(argc);

	for (int i = 0; i < argc; ++i)
	{
		args_[i] = argv[i];
		lc_args_[i] = StringHelper::to_lower_ascii(args_[i]);
	}
}

void ClArgs::unintialize()
{
	args_ = {};
	lc_args_ = {};
}

bool ClArgs::has_option(
	const std::string& option_name) const
{
	return find_option(option_name) >= 0;
}

int ClArgs::find_option(
	const std::string& option_name) const
{
	if (option_name.empty())
	{
		return -1;
	}

	const auto& string_helper = bstone::StringHelper{};
	const auto& lc_name = string_helper.to_lower_ascii(option_name);
	const auto arg_count = static_cast<int>(args_.size());

	for (int i = 0; i < arg_count; ++i)
	{
		const auto index = arg_count - i - 1;

		const auto& arg = args_[index];

		if (arg.size() != (2 + lc_name.size()))
		{
			continue;
		}

		if (arg.compare(0, 2, "--") != 0)
		{
			continue;
		}

		if (arg.compare(2, lc_name.size(), lc_name) == 0)
		{
			return index;
		}
	}

	return -1;
}

int ClArgs::find_argument(
	const std::string& name) const
{
	if (name.empty())
	{
		return -1;
	}

	const auto& string_helper = bstone::StringHelper{};
	const auto& lc_name = string_helper.to_lower_ascii(name);

	for (int i = 1; i < get_count(); ++i)
	{
		if (lc_name == args_[i])
		{
			return i;
		}
	}

	return -1;
}

int ClArgs::get_count() const
{
	return static_cast<int>(args_.size());
}

const std::string& ClArgs::get_argument(
	const int index) const
{
	if (index < 0 || index >= get_count())
	{
		const auto& string_helper = bstone::StringHelper{};

		return string_helper.get_empty();
	}

	return args_[index];
}

const std::string& ClArgs::get_option_value(
	const std::string& option_name) const
{
	auto option_index = find_option(option_name);

	if (option_index >= 0)
	{
		++option_index;
	}

	return get_argument(option_index);
}

void ClArgs::get_option_values(
	const std::string& option_name,
	std::string& value1,
	std::string& value2) const
{
	value1.clear();
	value2.clear();

	const auto option_index = find_option(option_name);

	if (option_index < 0)
	{
		return;
	}

	value1 = get_argument(option_index + 1);
	value2 = get_argument(option_index + 2);
}


} // bstone
