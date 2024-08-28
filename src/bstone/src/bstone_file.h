/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive.

#ifndef BSTONE_FILE_INCLUDED
#define BSTONE_FILE_INCLUDED

#include <cstdint>

#include "bstone_enum_flags.h"
#include "bstone_unique_resource.h"

namespace bstone {

using FileUResourceHandle =
#ifdef _WIN32
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
	void operator()(FileUResourceHandle handle) const noexcept;
};

using FileUResource = UniqueResource<
	FileUResourceHandle,
	FileUResourceDeleter,
	FileUResourceEmptyValue>;

// ==========================================================================

enum class FileOrigin
{
	none,
	begin,
	current,
	end,
};

// ==========================================================================

enum class FileOpenFlags : unsigned int
{
	none,
	create = 1U << 0, // Implies `write` mode.
	read = 1U << 1,
	write = 1U << 2,
	read_write = read | write,
	truncate = 1U << 3, // Implies `write` mode.
};

BSTONE_ENABLE_ENUM_CLASS_BITWISE_OPS_FOR(FileOpenFlags)

// ==========================================================================

enum class FileShareMode
{
	unrestricted,
	shared, // Requires a read access.
	exclusive, // Requires a write access.
};

// ==========================================================================

class File
{
public:
	File() noexcept;

	explicit File(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

	bool try_open(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

	void open(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

	void close() noexcept;
	bool is_open() const noexcept;
	std::intptr_t read(void* buffer, std::intptr_t count) const;
	void read_exactly(void* buffer, std::intptr_t count) const;
	std::intptr_t write(const void* buffer, std::intptr_t count) const;
	void write_exactly(const void* buffer, std::intptr_t count) const;
	std::int64_t seek(std::int64_t offset, FileOrigin origin) const;
	std::int64_t skip(std::int64_t offset) const;
	std::int64_t get_position() const;
	void set_position(std::int64_t position) const;
	std::int64_t get_size() const;
	void set_size(std::int64_t size) const;
	void flush() const;

private:
	enum class FileErrorMode
	{
		exception,
		error_code,
	};

private:
	FileUResource resource_{};

private:
	static void close_internal(FileUResource& resource) noexcept;

	static bool try_or_open_internal(
		const char* path,
		FileOpenFlags open_flags,
		FileShareMode share_mode,
		FileErrorMode file_error_mode,
		FileUResource& resource);
};

} // namespace bstone

#endif // BSTONE_FILE_INCLUDED
