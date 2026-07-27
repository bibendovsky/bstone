/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Special path primitives

#ifndef BSTONE_SYS_SPECIAL_PATH_INCLUDED
#define BSTONE_SYS_SPECIAL_PATH_INCLUDED

namespace bstone::sys {

struct SpecialPath
{
	/*
	Get the user-specific path where files can be written.

	Params:
	  - organization_name - organization name (optional, could be null).
	  - application_name - application name (required).
	  - buffer - a buffer to store a null-terminated path.
	  - buffer_size - buffer size.

	Returns:
	  Path length without null symbol.
	*/
	static int get_user_specific_data_path(
		const char* organization_name,
		const char* application_name,
		char* buffer,
		int buffer_size);

	/*
	Get the current user's home directory.

	Unlike the path above this is informational - a caller uses it to guess where
	another application keeps its data - so an unknown home directory is reported
	as a zero length instead of an error.

	Params:
	  - buffer - a buffer to store a null-terminated path, without a trailing separator.
	  - buffer_size - buffer size.

	Returns:
	  Path length without null symbol, or zero if the home directory is unknown or
	  does not fit into the buffer.
	*/
	static int get_home_path(char* buffer, int buffer_size) noexcept;
};

} // namespace bstone::sys

#endif // BSTONE_SYS_SPECIAL_PATH_INCLUDED
