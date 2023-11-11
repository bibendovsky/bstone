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

enum class Win32FileErrorCode
{
	none = 0,
	unknown,
	file_not_found,
	not_regular_file,
	failed_to_truncate
};

BSTONE_CXX_NODISCARD Win32FileErrorCode file_open(
	const char* file_name,
	FileOpenMode open_mode,
	FileUResource& resource) noexcept
{
	const auto is_create = (open_mode & FileOpenMode::create) != FileOpenMode::none;
	const auto is_truncate = (open_mode & FileOpenMode::truncate) != FileOpenMode::none;
	const auto is_readable = (open_mode & FileOpenMode::read) != FileOpenMode::none;
	const auto is_writable = is_create || is_truncate || (open_mode & FileOpenMode::write) != FileOpenMode::none;

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

	//
	auto is_manual_truncation = false;
	auto win32_create_disposition = DWORD{};

	if (is_create)
	{
		if (is_truncate)
		{
			is_manual_truncation = true;
		}

		win32_create_disposition = OPEN_ALWAYS;
	}
	else
	{
		if (is_truncate)
		{
			win32_create_disposition = TRUNCATE_EXISTING;
		}
		else
		{
			win32_create_disposition = OPEN_EXISTING;
		}
	}

	//
	{
		const auto u16_file_name = Win32WString{file_name};

		resource.reset(CreateFileW(
			u16_file_name.get_data(),
			win32_desired_access,
			FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
			nullptr,
			win32_create_disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr));
	}

	if (resource.is_empty())
	{
		const auto error_code = GetLastError();

		switch (error_code)
		{
			case ERROR_FILE_NOT_FOUND: return Win32FileErrorCode::file_not_found;
			default: return Win32FileErrorCode::unknown;
		}
	}

	const auto win32_file_type = GetFileType(resource.get());

	if (win32_file_type != FILE_TYPE_DISK)
	{
		return Win32FileErrorCode::not_regular_file;
	}

	if (is_manual_truncation)
	{
		const auto truncate_result = SetEndOfFile(resource.get());

		if (truncate_result == 0)
		{
			return Win32FileErrorCode::failed_to_truncate;
		}
	}

	return Win32FileErrorCode::none;
}

} // namespace

// ==========================================================================

FileUResourceHandle FileUResourceEmptyValue::operator()() const noexcept
{
	return INVALID_HANDLE_VALUE;
}

void FileUResourceDeleter::operator()(FileUResourceHandle handle) const
{
	CloseHandle(static_cast<HANDLE>(handle));
}

// ==========================================================================

File::File(const char* file_name, FileOpenMode open_mode)
{
	open(file_name, open_mode);
}

BSTONE_CXX_NODISCARD bool File::try_open(const char* file_name, FileOpenMode open_mode) noexcept
{
	close();

	auto resource = FileUResource{};
	const auto win32_error_code = file_open(file_name, open_mode, resource);

	if (win32_error_code != Win32FileErrorCode::none)
	{
		return false;
	}

	resource_.swap(resource);
	return true;
}

void File::open(const char* file_name, FileOpenMode open_mode)
{
	close();

	auto resource = FileUResource{};
	const auto error_code = file_open(file_name, open_mode, resource);

	switch (error_code)
	{
		case Win32FileErrorCode::none: resource_.swap(resource); break;
		case Win32FileErrorCode::file_not_found: BSTONE_THROW_STATIC_SOURCE("Not found.");
		case Win32FileErrorCode::not_regular_file: BSTONE_THROW_STATIC_SOURCE("Not a regular file.");
		case Win32FileErrorCode::failed_to_truncate: BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
		default: BSTONE_THROW_STATIC_SOURCE("Failed to open.");
	}
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

	if (read_file_result == FALSE)
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

	if (write_file_result == FALSE)
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

	LARGE_INTEGER win32_offset;
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

BSTONE_CXX_NODISCARD std::int64_t File::get_size() const
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

	if (set_file_pointer_ex_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get current position.");
	}

	win32_offset.QuadPart = size;
	set_file_pointer_ex_result = SetFilePointerEx(resource_.get(), win32_offset, nullptr, FILE_BEGIN);

	if (set_file_pointer_ex_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to set position at new size.");
	}

	const auto set_end_of_file_result = SetEndOfFile(resource_.get());

	if (set_end_of_file_result == FALSE)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to resize.");
	}

	win32_offset.QuadPart = size;
	set_file_pointer_ex_result = SetFilePointerEx(resource_.get(), win32_position, nullptr, FILE_BEGIN);

	if (set_file_pointer_ex_result == FALSE)
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

} // namespace bstone

#endif // _WIN32
