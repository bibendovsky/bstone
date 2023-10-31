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

#include <cstdint>

#include <algorithm>
#include <limits>
#include <type_traits>

#include "bstone_exception.h"
#include "bstone_file.h"

static_assert(std::is_signed<off_t>::value && (sizeof(off_t) == 4 || sizeof(off_t) == 8), "Unsupported type.");

namespace bstone {

namespace {

constexpr auto file_posix_supports_64_bit_size = sizeof(off_t) == 8;

constexpr auto file_posix_max_int = std::min(
	std::int64_t{std::numeric_limits<off_t>::max()},
	std::int64_t{std::numeric_limits<std::intptr_t>::max()});

} // namespace

// ==========================================================================

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return -1;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const
{
#if !defined(NDEBUG)
	const auto posix_result =
#endif
		close(handle);
#if !defined(NDEBUG)
	assert(posix_result == 0);
#endif
}

// ==========================================================================

File::File(const char* file_name, FileOpenMode open_mode)
{
	open(file_name, open_mode);
}

void File::open(const char* file_name, FileOpenMode open_mode)
{
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
}

std::intptr_t File::read(void* buffer, std::intptr_t count) const
{
	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_read = static_cast<size_t>(std::min(count, file_posix_max_int));
	const auto posix_number_of_bytes_read = ::read(posix_file_descriptor, buffer, posix_number_of_bytes_to_read);

	if (posix_number_of_bytes_read < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return static_cast<std::intptr_t>(posix_number_of_bytes_read);
}

std::intptr_t File::write(const void* buffer, std::intptr_t count) const
{
	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_write = static_cast<size_t>(std::min(count, file_posix_max_int));
	const auto posix_number_of_bytes_written = ::write(posix_file_descriptor, buffer, posix_number_of_bytes_to_write);

	if (posix_number_of_bytes_written < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return static_cast<std::intptr_t>(posix_number_of_bytes_written);
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	if (!file_posix_supports_64_bit_size)
	{
		if (std::abs(offset) > file_posix_max_int)
		{
			BSTONE_THROW_STATIC_SOURCE("Offset out of range.");
		}
	}

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

	return static_cast<std::int64_t>(lseek_result);
}

std::int64_t File::get_size() const
{
	struct stat posix_stat;
	const auto fstat_result = fstat(resource_.get(), &posix_stat);

	if (fstat_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get a size.");
	}

	return posix_stat.st_size;
}

void File::set_size(std::int64_t size) const
{
	if (!file_posix_supports_64_bit_size)
	{
		if (std::abs(size) > file_posix_max_int)
		{
			BSTONE_THROW_STATIC_SOURCE("Size out of range.");
		}
	}

	const auto ftruncate_result = ftruncate(resource_.get(), size);

	if (ftruncate_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
	}
}

void File::flush() const
{
	const auto fsync_result = fsync(resource_.get());

	if (fsync_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
}

} // namespace bstone

#endif // _WIN32
