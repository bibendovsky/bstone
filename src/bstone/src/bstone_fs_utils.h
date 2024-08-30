/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File system utils.

#ifndef BSTONE_FS_UTILS_INCLUDED
#define BSTONE_FS_UTILS_INCLUDED

#include <string>

namespace bstone {
namespace fs_utils {

std::string normalize_path(const std::string& path);
std::string append_path_separator(const std::string& path);
std::string append_path(const std::string& path, const std::string& sub_path);
void replace_extension(std::string& path_name, const std::string& new_extension);
std::string get_working_dir();
void rename_with_overwrite(const std::string& old_path, const std::string& new_path);

} // fs_utils
} // bstone

#endif // BSTONE_FS_UTILS_INCLUDED
