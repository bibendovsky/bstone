/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (Win32).

#ifdef _WIN32

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "bstone_file.h"

#include <cstdint>

#include <windows.h>

#include "bstone_exception.h"
#include "bstone_win32_wstring.h"

namespace bstone {

namespace {

constexpr auto win32_file_max_count = std::intptr_t{0x7FFFF000};

} // namespace

// ==========================================================================

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return INVALID_HANDLE_VALUE;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const noexcept
{
	CloseHandle(static_cast<HANDLE>(handle));
}

// ==========================================================================

File::File(const char* path, FileOpenFlags open_flags)
{
	try_or_open_internal(path, open_flags, false, resource_);
}

bool File::try_open(const char* path, FileOpenFlags open_flags)
{
	return try_or_open_internal(path, open_flags, true, resource_);
}

void File::open(const char* path, FileOpenFlags open_flags)
{
	try_or_open_internal(path, open_flags, false, resource_);
}

std::intptr_t File::read(void* buffer, std::intptr_t count) const
{
	ensure_is_open();
	ensure_buffer_not_null(buffer);
	ensure_count_not_negative(count);

	const auto win32_handle = resource_.get();
	const auto win32_size_to_read = static_cast<DWORD>(std::min(count, win32_file_max_count));
	auto win32_read_size = DWORD{};

	const auto win32_result = ReadFile(
		win32_handle,
		buffer,
		win32_size_to_read,
		&win32_read_size,
		nullptr);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return static_cast<std::intptr_t>(win32_read_size);
}

std::intptr_t File::write(const void* buffer, std::intptr_t count) const
{
	ensure_is_open();
	ensure_buffer_not_null(buffer);
	ensure_count_not_negative(count);

	const auto win32_handle = resource_.get();
	const auto win32_size_to_write = static_cast<DWORD>(std::min(count, win32_file_max_count));
	auto win32_written_size = DWORD{};

	const auto win32_result = WriteFile(
		win32_handle,
		buffer,
		win32_size_to_write,
		&win32_written_size,
		nullptr);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return static_cast<std::intptr_t>(win32_written_size);
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	ensure_is_open();

	auto win32_origin = DWORD{};

	switch (origin)
	{
		case FileOrigin::begin: win32_origin = FILE_BEGIN; break;
		case FileOrigin::current: win32_origin = FILE_CURRENT; break;
		case FileOrigin::end: win32_origin = FILE_END; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	auto win32_offset = LARGE_INTEGER{};
	win32_offset.QuadPart = offset;
	auto win32_position = LARGE_INTEGER{};

	const auto win32_result = SetFilePointerEx(
		resource_.get(),
		win32_offset,
		&win32_position,
		win32_origin);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position.");
	}

	return win32_position.QuadPart;
}

std::int64_t File::get_size() const
{
	ensure_is_open();

	auto win32_size = LARGE_INTEGER{};
	const auto win32_result = GetFileSizeEx(resource_.get(), &win32_size);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get size.");
	}

	return win32_size.QuadPart;
}

void File::set_size(std::int64_t size) const
{
	ensure_is_open();
	ensure_size_not_negative(size);

	auto win32_result = BOOL{};
	auto win32_offset = LARGE_INTEGER{};
	auto win32_position = LARGE_INTEGER{};

	win32_result = SetFilePointerEx(resource_.get(), win32_offset, &win32_position, FILE_CURRENT);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current position.");
	}

	win32_offset.QuadPart = size;
	win32_result = SetFilePointerEx(resource_.get(), win32_offset, nullptr, FILE_BEGIN);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position at new size.");
	}

	win32_result = SetEndOfFile(resource_.get());

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to resize.");
	}

	win32_offset.QuadPart = size;
	win32_result = SetFilePointerEx(resource_.get(), win32_position, nullptr, FILE_BEGIN);

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to return to saved position.");
	}
}

void File::flush() const
{
	ensure_is_open();

	const auto win32_result = FlushFileBuffers(resource_.get());

	if (win32_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
}

bool File::try_or_open_internal(
	const char* path,
	FileOpenFlags open_flags,
	bool ignore_file_errors,
	FileUResource& resource)
{
	// Release previous resource.
	//

	close_internal(resource);

	// Validate input parameters.
	//

	ensure_path_not_null(path);

	const auto is_create = (open_flags & FileOpenFlags::create) != FileOpenFlags::none;
	const auto is_truncate = (open_flags & FileOpenFlags::truncate) != FileOpenFlags::none;
	const auto is_readable = (open_flags & FileOpenFlags::read) != FileOpenFlags::none;

	const auto is_writable = is_create || is_truncate ||
		(open_flags & FileOpenFlags::write) != FileOpenFlags::none;

	if (!is_readable && !is_writable)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid open flags.");
	}

	// Make desired access.
	//

	auto win32_desired_access = DWORD{};

	if (is_readable)
	{
		win32_desired_access |= GENERIC_READ;
	}

	if (is_writable)
	{
		win32_desired_access |= GENERIC_WRITE;
	}

	// Make sharing mode.
	//

	constexpr auto win32_share_mode = DWORD{FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE};

	// Make disposition.
	//

	auto should_truncate = false;
	auto win32_creation_disposition = DWORD{};

	if (is_create)
	{
		if (is_truncate)
		{
			should_truncate = true;
		}

		win32_creation_disposition = OPEN_ALWAYS;
	}
	else
	{
		if (is_truncate)
		{
			win32_creation_disposition = TRUNCATE_EXISTING;
		}
		else
		{
			win32_creation_disposition = OPEN_EXISTING;
		}
	}

	// Make a resource.
	//

	auto new_resource = FileUResource{};

	{
		const auto u16_file_name = Win32WString{path};

		new_resource.reset(CreateFileW(
			u16_file_name.get_data(),
			win32_desired_access,
			win32_share_mode,
			nullptr,
			win32_creation_disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr));
	}

	if (new_resource.is_empty())
	{
		if (ignore_file_errors)
		{
			return false;
		}

		const auto win32_last_error = GetLastError();

		switch (win32_last_error)
		{
			case ERROR_FILE_NOT_FOUND: BSTONE_THROW_STATIC_SOURCE("Not found.");
			case ERROR_ACCESS_DENIED: BSTONE_THROW_STATIC_SOURCE("Access denied.");
			default: BSTONE_THROW_STATIC_SOURCE("Failed to open.");
		}
	}

	// Validate a type.
	//

	const auto win32_file_type = GetFileType(new_resource.get());

	if (win32_file_type != FILE_TYPE_DISK)
	{
		if (ignore_file_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Not a regular file.");
	}

	// Truncate if necessary.
	//

	if (should_truncate)
	{
		const auto win32_result = SetEndOfFile(new_resource.get());

		if (win32_result == FALSE)
		{
			if (ignore_file_errors)
			{
				return false;
			}

			BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
		}
	}

	// Commit changes.
	//

	resource.swap(new_resource);
	return true;
}

} // namespace bstone

#endif // _WIN32
