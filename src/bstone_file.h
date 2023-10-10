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
#include "bstone_unique_resource.h"

namespace bstone {

using FileUResourceHandle =
#if defined(_WIN32)
	void*
#else
	int
#endif
;

struct FileUResourceEmptyValue
{
	FileUResourceHandle operator()() const noexcept;
};

struct FileUResourceDeleter
{
	void operator()(FileUResourceHandle handle) const;
};

using FileUResource = UniqueResource<
	FileUResourceHandle,
	FileUResourceDeleter,
	FileUResourceEmptyValue>;

// ==========================================================================

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
	explicit File(const char* file_name, FileOpenMode open_mode = FileOpenMode::read);

	void open(const char* file_name, FileOpenMode open_mode = FileOpenMode::read);
	void close();
	bool is_open() const;
	IntP read(void* buffer, IntP count) const;
	void read_exact(void* buffer, IntP count) const;
	IntP write(const void* buffer, IntP count) const;
	void write_exact(const void* buffer, IntP count) const;
	Int64 seek(Int64 offset, FileOrigin origin) const;
	Int64 skip(Int64 delta) const;
	Int64 get_position() const;
	void set_position(Int64 position) const;
	Int64 get_size() const;
	void set_size(Int64 size) const;
	void flush() const;

private:
	FileUResource resource_{};
};

} // namespace bstone

#endif // BSTONE_FILE_INCLUDED
