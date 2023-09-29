/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (POSIX).

#if !defined(_WIN32)

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <limits>
#include <type_traits>

#include "bstone_exception.h"
#include "bstone_file.h"
#include "bstone_int.h"

static_assert(std::is_signed<off_t>::value && (sizeof(off_t) == 4 || sizeof(off_t) == 8), "Unsupported type.");

namespace bstone {

namespace {

constexpr auto file_posix_is_64_bit = sizeof(off_t) == 8;

constexpr auto file_posix_max_read_write_size = std::min(
	Int64{std::numeric_limits<off_t>::max()},
	Int64{std::numeric_limits<IntP>::max()});

} // namespace

// ==========================================================================

File::File(const char* file_name, FileOpenMode open_mode)
try {
	open(file_name, open_mode);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void File::open(const char* file_name, FileOpenMode open_mode)
try {
	close();

	const auto is_create = (open_mode & FileOpenMode::create) != FileOpenMode::none;
	const auto is_truncate = (open_mode & FileOpenMode::truncate) != FileOpenMode::none;
	const auto is_readable = (open_mode & FileOpenMode::read) != FileOpenMode::none;
	const auto is_writable = is_create || is_truncate || (open_mode & FileOpenMode::write) != FileOpenMode::none;

	auto posix_oflag = 0;
	auto posix_mode = mode_t{};

	if (is_create)
	{
		posix_oflag |= O_CREAT;
		posix_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	}

	if (is_truncate)
	{
		posix_oflag |= O_TRUNC;
	}

	if (is_readable && is_writable)
	{
		posix_oflag |= O_RDWR;
	}
	else if (is_readable)
	{
		posix_oflag |= O_RDONLY;
	}
	else if (is_writable)
	{
		posix_oflag |= O_WRONLY;
	}

	auto resource = FileUResource{::open(file_name, posix_oflag, posix_mode)};

	if (resource.is_empty())
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to open.");
	}

	struct stat posix_stat;
	const auto fstat_result = fstat(resource.get(), &posix_stat);

	if (fstat_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get stat.");
	}

	if (!S_ISREG(posix_stat.st_mode))
	{
		BSTONE_THROW_STATIC_SOURCE("Expected a regualr file.");
	}

	resource_.swap(resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

IntP File::read(void* buffer, IntP count) const
try {
	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_read = static_cast<size_t>(std::min(count, file_posix_max_read_write_size));
	const auto posix_number_of_bytes_read = ::read(posix_file_descriptor, buffer, posix_number_of_bytes_to_read);

	if (posix_number_of_bytes_read < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return static_cast<IntP>(posix_number_of_bytes_read);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

IntP File::write(const void* buffer, IntP count) const
try {
	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_write = static_cast<size_t>(std::min(count, file_posix_max_read_write_size));
	const auto posix_number_of_bytes_written = ::write(posix_file_descriptor, buffer, posix_number_of_bytes_to_write);

	if (posix_number_of_bytes_written < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return static_cast<IntP>(posix_number_of_bytes_written);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::get_position() const
try {
	const auto lseek_result = lseek(resource_.get(), 0, SEEK_CUR);

	if (lseek_result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get position.");
	}

	return static_cast<Int64>(lseek_result);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::set_position(Int64 offset, FileOrigin origin) const
try {
	int posix_origin;

	switch (origin)
	{
		case FileOrigin::begin: posix_origin = SEEK_SET; break;
		case FileOrigin::current: posix_origin = SEEK_CUR; break;
		case FileOrigin::end: posix_origin = SEEK_END; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown type.");
	}

	const auto lseek_result = lseek(resource_.get(), offset, posix_origin);

	if (lseek_result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position.");
	}

	return static_cast<Int64>(lseek_result);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::get_size() const
try {
	struct stat posix_stat;
	const auto fstat_result = fstat(resource_.get(), &posix_stat);

	if (fstat_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get a size.");
	}

	return static_cast<Int64>(posix_stat.st_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void File::set_size(Int64 size) const
try {
	const auto ftruncate_result = ftruncate(resource_.get(), size);

	if (ftruncate_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void File::flush() const
try {
	const auto fsync_result = fsync(resource_.get());

	if (fsync_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

#endif // _WIN32
