/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#if !defined(BSTONE_FILE_INCLUDED)
#define BSTONE_FILE_INCLUDED

#include "bstone_enum_flags.h"
#include "bstone_int.h"
#include "bstone_file_uresource.h"

namespace bstone {

enum class FileOrigin
{
	none = 0,

	begin,
	current,
	end,
};

enum class FileOpenMode : unsigned int
{
	none = 0,

	create = 1U << 0, // Implies `write` mode.
	read = 1U << 1,
	write = 1U << 2,
	truncate = 1U << 3, // Implies `write` mode.

	read_write = read | write,
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(FileOpenMode)

// ==========================================================================

class File
{
public:
	File() = default;
	File(const char* file_name, FileOpenMode open_mode);
	File(const char* file_name); // Opens an existing file for reading.

	void open(const char* file_name, FileOpenMode open_mode);
	void open(const char* file_name); // Opens an existing file for reading.
	void close();
	bool is_open() const;
	IntP read(void* buffer, IntP count) const;
	IntP write(const void* buffer, IntP count) const;
	Int64 get_position() const;
	Int64 set_position(Int64 offset, FileOrigin origin) const;
	Int64 get_size() const;
	void set_size(Int64 size) const;
	void flush() const;

	static bool supports_64_bit_size();

private:
	FileUResource resource_{};
};

} // namespace bstone

#endif // BSTONE_FILE_INCLUDED
