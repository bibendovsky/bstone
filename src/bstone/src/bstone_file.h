/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#ifndef BSTONE_FILE_INCLUDED
#define BSTONE_FILE_INCLUDED

#include <stdint.h>

// ==========================================================================

namespace bstone {

enum FileErrorCode
{
	ec_file_none = 0,
	ec_file_unknown = -1,
	ec_file_out_of_memory = -3,
	ec_file_invalid_arguments = -4,

	ec_file_open = -100, // Could not open a file.
	ec_file_not_found = -101, // File or path not found.
	ec_file_access_denied = -102, // Access denied to file or path.
	ec_file_utf8_to_native = -103, // Could not convert a file path to native encoding.
	ec_file_not_regular = -104, // Not a regular file.
	ec_file_truncate = -105, // Could not truncate a file.
	ec_file_lock = -106 // Could not lock a file.
};

// ==========================================================================

enum FileOrigin
{
	file_origin_none = 0,
	file_origin_begin = 1,
	file_origin_current = 2,
	file_origin_end = 3,
};

// ==========================================================================

enum FileFlags
{
	file_flags_none = 0,
	file_flags_read = 1 << 0,
	file_flags_write = 1 << 1,
	file_flags_create = 1 << 2, // Implies "write" flag.
	file_flags_truncate = 1 << 3, // Implies "write" flag.
	file_flags_shared = 1 << 4, // Implies "read" flag. Mutually exclusive.
	file_flags_exclusive = 1 << 5, // Implies "write" flag. Mutually exclusive.

	file_flags_read_write = file_flags_read | file_flags_write,
};

// --------------------------------------------------------------------------

FileFlags operator|(FileFlags a, FileFlags b);
FileFlags& operator|=(FileFlags& a, FileFlags b);

// ==========================================================================

enum FileLockType
{
	file_lock_unknown = 0,
	file_lock_shared = 1,
	file_lock_exclusive = 2,
};

// ==========================================================================

class File
{
public:
	static const int invalid_handle = -1;

public:
	File();
	File(const char* path, FileFlags flags);
	File(const char* path, FileFlags flags, FileErrorCode& error_code);
	File(File&& rhs) noexcept;
	File& operator=(File&& rhs) noexcept;
	~File();

	bool is_open() const;
	bool open(const char* path, FileFlags flags);
	bool open(const char* path, FileFlags flags, FileErrorCode& error_code);
	void close();
	intptr_t read(void* buffer, intptr_t size) const;
	bool read_exactly(void* buffer, intptr_t size) const;
	intptr_t write(const void* buffer, intptr_t size) const;
	bool write_exactly(const void* buffer, intptr_t size) const;
	int64_t seek(int64_t offset, FileOrigin origin) const;
	int64_t skip(int64_t offset) const;
	int64_t get_position() const;
	bool set_position(int64_t position) const;
	int64_t get_size() const;
	bool set_size(int64_t size) const;
	bool flush() const;
	bool lock(FileLockType lock_type) const;
	bool lock_shared() const;
	bool lock_exclusive() const;
	bool unlock() const;
	void swap(File& file);

private:
	File(const File&) = delete;
	File& operator=(const File&) = delete;

private:
	int handle_;
};

} // namespace bstone

#endif // BSTONE_FILE_INCLUDED
