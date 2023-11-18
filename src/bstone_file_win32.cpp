/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (WIN32).

#if defined(_WIN32)

#if !defined(WIN32_LEAN_AND_MEAN)
	#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(NOMINMAX)
	#define NOMINMAX
#endif

#include <cstdint>

#include <windows.h>

#include "bstone_assert.h"
#include "bstone_exception.h"
#include "bstone_file.h"
#include "bstone_win32_wstring.h"

namespace bstone {

namespace {

constexpr auto win32_file_max_read_write_size = std::intptr_t{0x7FFFFFFF};

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
	BSTONE_ASSERT(is_open());

	const auto win32_handle = resource_.get();
	const auto win32_size_to_read = static_cast<DWORD>(std::min(count, win32_file_max_read_write_size));
	DWORD win32_read_size;

	const auto read_file_result = ReadFile(
		win32_handle,
		buffer,
		win32_size_to_read,
		&win32_read_size,
		nullptr);

	if (read_file_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to read.");
	}

	return static_cast<std::intptr_t>(win32_read_size);
}

std::intptr_t File::write(const void* buffer, std::intptr_t count) const
{
	BSTONE_ASSERT(is_open());

	const auto win32_handle = resource_.get();
	const auto win32_size_to_write = static_cast<DWORD>(std::min(count, win32_file_max_read_write_size));
	DWORD win32_written_size;

	const auto write_file_result = WriteFile(
		win32_handle,
		buffer,
		win32_size_to_write,
		&win32_written_size,
		nullptr);

	if (write_file_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to write.");
	}

	return static_cast<std::intptr_t>(win32_written_size);
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	BSTONE_ASSERT(is_open());

	DWORD win32_origin;

	switch (origin)
	{
		case FileOrigin::begin: win32_origin = FILE_BEGIN; break;
		case FileOrigin::current: win32_origin = FILE_CURRENT; break;
		case FileOrigin::end: win32_origin = FILE_END; break;
		default: BSTONE_THROW_STATIC_SOURCE("Unknown origin.");
	}

	auto win32_offset = LARGE_INTEGER{};
	win32_offset.QuadPart = offset;
	LARGE_INTEGER win32_position;

	const auto set_file_pointer_ex_result = SetFilePointerEx(
		resource_.get(),
		win32_offset,
		&win32_position,
		win32_origin);

	if (set_file_pointer_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position.");
	}

	return win32_position.QuadPart;
}

std::int64_t File::get_size() const
{
	BSTONE_ASSERT(is_open());

	LARGE_INTEGER win32_size;
	const auto get_file_size_ex_result = GetFileSizeEx(resource_.get(), &win32_size);

	if (get_file_size_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get size.");
	}

	return win32_size.QuadPart;
}

void File::set_size(std::int64_t size) const
{
	BSTONE_ASSERT(is_open());

	auto win32_offset = LARGE_INTEGER{};
	LARGE_INTEGER win32_position;
	auto set_file_pointer_ex_result = SetFilePointerEx(resource_.get(), win32_offset, &win32_position, FILE_CURRENT);

	if (set_file_pointer_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current position.");
	}

	win32_offset.QuadPart = size;
	set_file_pointer_ex_result = SetFilePointerEx(resource_.get(), win32_offset, nullptr, FILE_BEGIN);

	if (set_file_pointer_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position at new size.");
	}

	const auto set_end_of_file_result = SetEndOfFile(resource_.get());

	if (set_end_of_file_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to resize.");
	}

	win32_offset.QuadPart = size;
	set_file_pointer_ex_result = SetFilePointerEx(resource_.get(), win32_position, nullptr, FILE_BEGIN);

	if (set_file_pointer_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to return to saved position.");
	}
}

void File::flush() const
{
	BSTONE_ASSERT(is_open());

	const auto flush_file_buffers_result = FlushFileBuffers(resource_.get());

	if (flush_file_buffers_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
}

bool File::try_or_open_internal(
	const char* path,
	FileOpenFlags open_flags,
	bool ignore_errors,
	FileUResource& resource)
{
	// Release previous resource.
	//

	close_internal(resource);

	// Validate input parameters.
	//

	const auto is_create = (open_flags & FileOpenFlags::create) != FileOpenFlags::none;
	const auto is_truncate = (open_flags & FileOpenFlags::truncate) != FileOpenFlags::none;
	const auto is_readable = (open_flags & FileOpenFlags::read) != FileOpenFlags::none;
	const auto is_writable = is_create || is_truncate || (open_flags & FileOpenFlags::write) != FileOpenFlags::none;

	if (!is_readable && !is_writable)
	{
		BSTONE_THROW_STATIC_SOURCE("Invalid open flags.");
	}

	// Make desired access.
	//

	auto desired_access = DWORD{};

	if (is_readable)
	{
		desired_access |= GENERIC_READ;
	}

	if (is_writable)
	{
		desired_access |= GENERIC_WRITE;
	}

	// Make sharing mode.
	//

	constexpr auto share_mode = DWORD{FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE};

	// Make disposition.
	//

	auto is_manual_truncation = false;
	auto creation_disposition = DWORD{};

	if (is_create)
	{
		if (is_truncate)
		{
			is_manual_truncation = true;
		}

		creation_disposition = OPEN_ALWAYS;
	}
	else
	{
		if (is_truncate)
		{
			creation_disposition = TRUNCATE_EXISTING;
		}
		else
		{
			creation_disposition = OPEN_EXISTING;
		}
	}

	// Make a resource.
	//

	auto new_resource = FileUResource{};

	{
		const auto u16_file_name = Win32WString{path};

		new_resource.reset(CreateFileW(
			u16_file_name.get_data(),
			desired_access,
			share_mode,
			nullptr,
			creation_disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr));
	}

	if (new_resource.is_empty())
	{
		if (ignore_errors)
		{
			return false;
		}

		const auto error_code = GetLastError();

		switch (error_code)
		{
			case ERROR_FILE_NOT_FOUND: BSTONE_THROW_STATIC_SOURCE("Not found.");
			case ERROR_ACCESS_DENIED: BSTONE_THROW_STATIC_SOURCE("Access denied.");
			default: BSTONE_THROW_STATIC_SOURCE("Failed to open.");
		}
	}

	// Validate a type.
	//

	const auto file_type = GetFileType(new_resource.get());

	if (file_type != FILE_TYPE_DISK)
	{
		if (ignore_errors)
		{
			return false;
		}

		BSTONE_THROW_STATIC_SOURCE("Not a regular file.");
	}

	// Truncate if necessary.
	//

	if (is_manual_truncation)
	{
		const auto truncate_result = SetEndOfFile(new_resource.get());

		if (truncate_result == 0)
		{
			if (ignore_errors)
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
