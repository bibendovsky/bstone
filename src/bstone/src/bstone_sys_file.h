/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive

#ifndef BSTONE_SYS_FILE_INCLUDED
#define BSTONE_SYS_FILE_INCLUDED

#include <cstdint>

namespace bstone::sys {

enum class FileOrigin
{
	none = 0,
	begin,
	current,
	end,
};

enum class FileMode
{
	none = 0,
	read, // Open an existing file for reading.
	create, // Create an empty file for writing.
};

class File
{
public:
	File() = default;
	File(const char* path, FileMode mode);
	File(const File&) = delete;
	File& operator=(const File&) = delete;
	File(File&& rhs) noexcept;
	File& operator=(File&& rhs) noexcept;
	~File();

	bool is_open() const;
	bool open(const char* path, FileMode mode);
	void close();
	int read(void* buffer, int size) const;
	bool read_exactly(void* buffer, int size) const;
	int write(const void* buffer, int size) const;
	bool write_exactly(const void* buffer, int size) const;
	std::int64_t seek(std::int64_t offset, FileOrigin origin) const;
	std::int64_t skip(std::int64_t offset) const;
	std::int64_t get_position() const;
	bool set_position(std::int64_t position) const;
	std::int64_t get_size() const;
	bool flush() const;

private:
	void* handle_{};
};

} // namespace bstone::sys

#endif // BSTONE_SYS_FILE_INCLUDED
