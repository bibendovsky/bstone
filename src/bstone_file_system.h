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
