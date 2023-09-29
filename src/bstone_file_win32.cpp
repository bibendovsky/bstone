/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (WIN32).

#if defined(_WIN32)

#if !defined(NOMINMAX)
	#define NOMINMAX
#endif

#include <windows.h>

#include "bstone_int.h"
#include "bstone_exception.h"
#include "bstone_file.h"
#include "bstone_win32_wstring.h"

namespace bstone {

namespace {

constexpr auto file_win32_max_read_write_size = IntP{0x7FFFFFFF};

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
	auto is_manual_truncate = false;
	auto win32_create_disposition = DWORD{};

	if (is_create)
	{
		if (is_truncate)
		{
			is_manual_truncate = true;
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
	auto resource = FileUResource{};

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
		BSTONE_THROW_STATIC_SOURCE("Failed to open.");
	}

	const auto win32_file_type = GetFileType(resource.get());

	if (win32_file_type != FILE_TYPE_DISK)
	{
		BSTONE_THROW_STATIC_SOURCE("Expected a regular file.");
	}

	if (is_manual_truncate)
	{
		const auto truncate_result = SetEndOfFile(resource.get());

		if (truncate_result == 0)
		{
			BSTONE_THROW_STATIC_SOURCE("Failed to truncate.");
		}
	}

	resource_.swap(resource);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

IntP File::read(void* buffer, IntP count) const
try {
	const auto win32_handle = resource_.get();
	const auto win32_size_to_read = static_cast<DWORD>(std::min(count, file_win32_max_read_write_size));
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

	return static_cast<IntP>(win32_read_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

IntP File::write(const void* buffer, IntP count) const
try {
	const auto win32_handle = resource_.get();
	const auto win32_size_to_write = static_cast<DWORD>(std::min(count, file_win32_max_read_write_size));
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

	return static_cast<IntP>(win32_written_size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::get_position() const
try {
	return set_position(0, FileOrigin::current);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::set_position(Int64 offset, FileOrigin origin) const
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

Int64 File::get_size() const
try {
	LARGE_INTEGER win32_size;
	const auto get_file_size_ex_result = GetFileSizeEx(resource_.get(), &win32_size);

	if (get_file_size_ex_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to get size.");
	}

	return win32_size.QuadPart;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void File::set_size(Int64 size) const
try {
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
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void File::flush() const
try {
	const auto flush_file_buffers_result = FlushFileBuffers(resource_.get());

	if (flush_file_buffers_result == 0)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to flush.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace bstone

#endif // _WIN32
