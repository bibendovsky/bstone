/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system utils.

#include "bstone_exception.h"
#include "bstone_fs.h"
#include "bstone_fs_utils.h"

namespace bstone {
namespace fs_utils {

std::string normalize_path(const std::string& path)
{
	auto result = path;

	for (auto& ch : result)
	{
		if ((ch == '/' || ch == '\\') && ch != fs::native_separator)
		{
			ch = fs::native_separator;
		}
	}

	return result;
}

std::string append_path_separator(const std::string& path)
{
	auto result = path;

	if (!result.empty() && result.back() != fs::native_separator)
	{
		result += fs::native_separator;
	}

	return result;
}

std::string append_path(const std::string& path, const std::string& sub_path)
{
	if (path.empty() && sub_path.empty())
	{
		return std::string{};
	}

	if (path.empty())
	{
		return sub_path;
	}
	else if (sub_path.empty())
	{
		return path;
	}
	else
	{
		auto result = std::string{};
		result.reserve(path.size() + sub_path.size() + 1);
		result.assign(path);

		if (result.back() != fs::native_separator)
		{
			result += fs::native_separator;
		}

		result += sub_path;
		return result;
	}
}

void replace_extension(std::string& path_name, const std::string& new_extension)
{
	if (path_name.empty() || new_extension.empty())
	{
		return;
	}

	if (new_extension.front() != '.')
	{
		BSTONE_THROW_STATIC_SOURCE("An extension should start with a dot.");
	}

	const auto separator_pos = path_name.find_last_of("\\/");
	const auto dot_pos = path_name.find('.', separator_pos);

	if (dot_pos != std::string::npos)
	{
		if (dot_pos == 0 || dot_pos == (path_name.size() - 1))
		{
			return;
		}

		path_name.resize(dot_pos);
	}

	path_name += new_extension;
}

std::string get_working_dir()
{
	constexpr auto max_path_size = 4096;
	auto working_directory = std::string{};
	working_directory.resize(max_path_size);
	const auto path_size = fs::get_working_directory(&working_directory.front(), max_path_size);
	working_directory.resize(path_size);
	return working_directory;
}

void rename_with_overwrite(const std::string& old_path, const std::string& new_path)
{
	fs::remove_if_exists(new_path.c_str());
	fs::rename(old_path.c_str(), new_path.c_str());
}

} // fs_utils
} // bstone
