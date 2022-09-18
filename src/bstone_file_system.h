/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// File system utils.
//


#ifndef BSTONE_FILE_SYSTEM_INCLUDED
#define BSTONE_FILE_SYSTEM_INCLUDED


#include <string>


namespace bstone
{
namespace file_system
{


std::string normalize_path(
	const std::string& path);

std::string append_path_separator(
	const std::string& path);

std::string append_path(
	const std::string& path,
	const std::string& sub_path);

void replace_extension(
	std::string& path_name,
	const std::string& new_extension);

std::string get_working_dir();

std::string resolve_path(
	const std::string& path);

bool has_file(
	const std::string& path);

void rename(
	const std::string& old_path,
	const std::string& new_path);


} // file_system
} // bstone


#endif // !BSTONE_FILE_SYSTEM_INCLUDED
