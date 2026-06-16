/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system utils.

#include "bstone_exception.h"
#include "bstone_fs_utils.h"
#include "bstone_sys_fs.h"

namespace bstone {
namespace fs_utils {

namespace {

std::size_t find_extension_position(std::string_view pathname)
{
	const std::size_t size = pathname.size();
	std::size_t extension_position = std::string_view::npos;
	for (std::size_t i = 0; i < size; ++i)
	{
		const std::size_t position = size - 1 - i;
		const char ch = pathname[position];
		if (ch == '.')
		{
			extension_position = position;
			break;
		}
		else if (ch == '/' || ch == '\\' || ch == ':')
			break;
	}
	if (extension_position != std::string_view::npos)
	{
		// Check for "dot" or "dot-dot" filename.
		const std::size_t size_left = size - extension_position;
		if (size_left == 1 || (size_left == 2 && pathname[extension_position + 1] == '.'))
			extension_position = std::string_view::npos;
	}
	return extension_position;
}

} // namespace

std::string normalize_path(const std::string& path)
{
	auto result = path;

	for (auto& ch : result)
	{
		if ((ch == '/' || ch == '\\') && ch != sys::native_separator)
		{
			ch = sys::native_separator;
		}
	}

	return result;
}

void normalize_separators_inplace(std::string& path)
{
	for (char& ch : path)
	{
		if (ch == '/' || ch == '\\')
			ch = sys::native_separator;
	}
}

std::string append_path_separator(const std::string& path)
{
	auto result = path;

	if (!result.empty() && result.back() != sys::native_separator)
	{
		result += sys::native_separator;
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

		if (result.back() != sys::native_separator)
		{
			result += sys::native_separator;
		}

		result += sub_path;
		return result;
	}
}

void append_path_inplace(std::string& path, std::string_view sub_path)
{
	if (sub_path.empty())
		return;
	if (path.empty())
	{
		path = sub_path;
		return;
	}
	const bool sub_path_starts_with_separator = (sub_path.starts_with('/') || sub_path.starts_with('\\'));
	const bool path_ends_with_separator = (path.ends_with('/') || path.ends_with('\\'));
	path.reserve(path.size() + sub_path.size() + 1);
	if (path_ends_with_separator && sub_path_starts_with_separator)
		path.append(sub_path.data() + 1, sub_path.size() - 1);
	else if (!path_ends_with_separator && !sub_path_starts_with_separator)
	{
		path += sys::native_separator;
		path += sub_path;
	}
	else
		path += sub_path;
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
	const auto path_size = sys::get_working_directory(&working_directory.front(), max_path_size);
	working_directory.resize(path_size);
	return working_directory;
}

void rename_with_overwrite(const std::string& old_path, const std::string& new_path)
{
	sys::remove_path_if_exists(new_path.c_str());
	sys::rename_path(old_path.c_str(), new_path.c_str());
}

bool ends_with_non_root_separator(const std::string& path)
{
	return path.size() > 1 && (path.ends_with('/') || path.ends_with('\\'));
}

std::string& trim_non_root_trailing_separator_inplace(std::string& pathname)
{
	if (ends_with_non_root_separator(pathname))
		pathname.resize(pathname.size() - 1);
	return pathname;
}

std::string get_dirname(std::string_view pathname)
{
	const std::size_t size = pathname.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		const std::size_t index = size - 1 - i;
		const char ch = pathname[index];
		if (ch == '/' || ch == '\\')
			return std::string{pathname.substr(0, index)};
		if (ch == ':')
			return std::string{pathname.substr(0, index + 1)};
	}
	return std::string{};
}

std::string_view get_filename(std::string_view pathname)
{
	const std::size_t size = pathname.size();
	for (std::size_t i = 0; i < size; ++i)
	{
		const std::size_t index = size - 1 - i;
		const char ch = pathname[index];
		if (ch == '/' || ch == '\\' || ch == ':')
			return pathname.substr(index + 1);
	}
	return pathname;
}

std::string_view get_extension(std::string_view pathname)
{
	if (const std::size_t extension_position = find_extension_position(pathname);
		extension_position != std::string_view::npos)
		return pathname.substr(extension_position);
	return std::string_view{};
}

void split_pathname_by_extension(std::string_view pathname, std::string_view& pathname_without_extension, std::string_view& extension)
{
	if (const std::size_t extension_position = find_extension_position(pathname);
		extension_position != std::string_view::npos)
	{
		pathname_without_extension = pathname.substr(0, extension_position);
		extension = pathname.substr(extension_position);
	}
	else
	{
		pathname_without_extension = pathname;
		extension = std::string_view{};
	}
}

} // fs_utils
} // bstone
