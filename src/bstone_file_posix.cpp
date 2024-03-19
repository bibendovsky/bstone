/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (POSIX).

#ifndef _WIN32

#include "bstone_file.h"

#include <cstdint>

#include <algorithm>
#include <limits>
#include <type_traits>

#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "bstone_exception.h"

static_assert(
	std::is_signed<::off_t>::value && (sizeof(::off_t) == 4 || sizeof(::off_t) == 8),
	"Unsupported type \"off_t\".");

namespace bstone {

namespace {

constexpr auto posix_file_supports_64_bit_size = sizeof(::off_t) == 8;

constexpr auto posix_file_max_int = std::min(
	std::int64_t{std::numeric_limits<::off_t>::max()},
	std::int64_t{std::numeric_limits<std::intptr_t>::max()});

} // namespace

// ==========================================================================

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return -1;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const noexcept
{
	::close(handle);
}

// ==========================================================================

std::intptr_t File::read(void* buffer, std::intptr_t count) const
{
	ensure_is_open();
	validate_buffer(buffer);
	validate_count(count);

	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_read = static_cast<::size_t>(
		std::min(count, static_cast<std::intptr_t>(posix_file_max_int)));
	const auto posix_number_of_bytes_read = ::read(
		posix_file_descriptor, buffer, posix_number_of_bytes_to_read);

	if (posix_number_of_bytes_read < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return static_cast<std::intptr_t>(posix_number_of_bytes_read);
}

std::intptr_t File::write(const void* buffer, std::intptr_t count) const
{
	ensure_is_open();
	validate_buffer(buffer);
	validate_count(count);

	const auto posix_file_descriptor = resource_.get();
	const auto posix_number_of_bytes_to_write = static_cast<::size_t>(
		std::min(count, static_cast<std::intptr_t>(posix_file_max_int)));
	const auto posix_number_of_bytes_written = ::write(
		posix_file_descriptor, buffer, posix_number_of_bytes_to_write);

	if (posix_number_of_bytes_written < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return static_cast<std::intptr_t>(posix_number_of_bytes_written);
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	ensure_is_open();

	if (!posix_file_supports_64_bit_size)
	{
		if (std::abs(offset) > posix_file_max_int)
		{
			BSTONE_THROW_STATIC_SOURCE("Offset out of range.");
		}
	}

	auto posix_origin = 0;

	switch (origin)
	{
		case FileOrigin::begin: posix_origin = SEEK_SET; break;
		case FileOrigin::current: posix_origin = SEEK_CUR; break;
		case FileOrigin::end: posix_origin = SEEK_END; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	const auto lseek_result = ::lseek(resource_.get(), offset, posix_origin);

	if (lseek_result < 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position.");
	}

	return static_cast<std::int64_t>(lseek_result);
}

std::int64_t File::get_size() const
{
	ensure_is_open();

	struct ::stat posix_stat{};
	const auto fstat_result = ::fstat(resource_.get(), &posix_stat);

	if (fstat_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get a size.");
	}

	return posix_stat.st_size;
}

void File::set_size(std::int64_t size) const
{
	ensure_is_open();
	validate_size(size);

	if (!posix_file_supports_64_bit_size)
	{
		if (std::abs(size) > posix_file_max_int)
		{
			BSTONE_THROW_STATIC_SOURCE("Size out of range.");
		}
	}

	const auto ftruncate_result = ::ftruncate(resource_.get(), size);

	if (ftruncate_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
	}
}

void File::flush() const
{
	ensure_is_open();

	const auto fsync_result = ::fsync(resource_.get());

	if (fsync_result != 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
}

bool File::try_or_open_internal(
	const char* path,
	FileOpenFlags open_flags,
	FileErrorMode file_error_mode,
	FileUResource& resource)
{
	// Release previous resource.
	//

	close_internal(resource);

	// Validate input parameters.
	//

	validate_path(path);

	const auto is_create = (open_flags & FileOpenFlags::create) != FileOpenFlags::none;
	const auto is_truncate = (open_flags & FileOpenFlags::truncate) != FileOpenFlags::none;
	const auto is_readable = (open_flags & FileOpenFlags::read) != FileOpenFlags::none;
	const auto is_writable = is_create || is_truncate ||
		(open_flags & FileOpenFlags::write) != FileOpenFlags::none;

	if (!is_readable && !is_writable)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid flags combination.");
	}

	const auto use_error_code = file_error_mode == FileErrorMode::error_code;

	// Make status flags, access mode and access permission bits.
	//

	auto oflag = 0;
	auto mode = ::mode_t{};

	if (is_create)
	{
		oflag |= O_CREAT;
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	}

	if (is_truncate)
	{
		oflag |= O_TRUNC;
	}

	if (is_readable && is_writable)
	{
		oflag |= O_RDWR;
	}
	else if (is_readable)
	{
		oflag |= O_RDONLY;
	}
	else if (is_writable)
	{
		oflag |= O_WRONLY;
	}

	// Make a resource.
	//

	auto new_resource = FileUResource{::open(path, oflag, mode)};

	if (new_resource.is_empty())
	{
		if (use_error_code)
		{
			return false;
		}

		switch (errno)
		{
			case ENOENT: BSTONE_THROW_STATIC_SOURCE("Not found.");
			case EACCES: BSTONE_THROW_STATIC_SOURCE("Access denied.");
			default: BSTONE_THROW_STATIC_SOURCE("Failed to open.");
		}
	}

	// Validate a type.
	//

	struct ::stat posix_stat{};
	const auto fstat_result = ::fstat(new_resource.get(), &posix_stat);

	if (fstat_result != 0)
	{
		if (use_error_code)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Failed to get status.");
	}

	if (!S_ISREG(posix_stat.st_mode))
	{
		if (use_error_code)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Not a regular file.");
	}

	// Commit changes.
	//

	resource.swap(new_resource);
	return true;
}

} // namespace bstone

#endif // _WIN32
