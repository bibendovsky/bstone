/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (Win32).

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <limits.h>
#include <stdlib.h>
#include <algorithm>
#include <windows.h>
#include "bstone_assert.h"
#include "bstone_file.h"

// ==========================================================================

namespace bstone {

namespace {

const int file_win32_stack_path_max_length = 511;
const int file_win32_stack_path_max_length_with_nul = file_win32_stack_path_max_length + 1;
const int file_win32_path_max_stack_bytes = sizeof(WCHAR) * file_win32_stack_path_max_length_with_nul;

template<bool TIsSameSize>
struct FileWin32MaxReadWriteSizeT
{
	static const intptr_t value = INTPTR_MAX;
};

template<>
struct FileWin32MaxReadWriteSizeT<true>
{
	static const intptr_t value = INT32_MAX;
};

const intptr_t file_win32_max_read_write_size =
	FileWin32MaxReadWriteSizeT<sizeof(intptr_t) == sizeof(DWORD)>::value;

template<typename T>
void file_win32_ignore_result(const T&) {}

template<bool TIsValid>
class FileWin32CastT {};

template<>
class FileWin32CastT<true>
{
public:
	static HANDLE int_to_handle(int handle)
	{
		return reinterpret_cast<HANDLE>(static_cast<INT_PTR>(handle));
	}

	static int handle_to_int(HANDLE handle)
	{
		return static_cast<int>(reinterpret_cast<INT_PTR>(handle));
	}
};

typedef FileWin32CastT<sizeof(int) == sizeof(INT32)> FileWin32Cast;

// --------------------------------------------------------------------------

HANDLE file_win32_cast_int_to_handle(int handle)
{
	return reinterpret_cast<HANDLE>(static_cast<INT_PTR>(handle));
}

int file_win32_cast_handle_to_int(HANDLE handle)
{
	return static_cast<int>(reinterpret_cast<INT_PTR>(handle));
}

void file_win32_closehandle(HANDLE win32_handle)
{
	const BOOL win32_result = CloseHandle(win32_handle);
	BSTONE_ASSERT(win32_result);
	file_win32_ignore_result(win32_result);
}

void file_win32_closehandle(int handle)
{
	file_win32_closehandle(file_win32_cast_int_to_handle(handle));
}

bool file_win32_lockfileex(HANDLE win32_handle, DWORD win32_lock_flags)
{
	OVERLAPPED win32_offset = OVERLAPPED();
	return LockFileEx(win32_handle, win32_lock_flags, 0, UINT32_MAX, UINT32_MAX, &win32_offset);
}

bool file_win32_unlockfile(HANDLE win32_handle)
{
	return UnlockFile(win32_handle, 0, 0, UINT32_MAX, UINT32_MAX);
}

bool file_win32_createfilew(
	const WCHAR* win32_path,
	DWORD win32_desired_access,
	DWORD win32_share_mode,
	DWORD win32_creation_disposition,
	DWORD win32_flags_and_attributes,
	HANDLE& win32_handle,
	FileErrorCode& error_code)
{
	win32_handle = CreateFileW(
		win32_path, // lpFileName
		win32_desired_access, // dwDesiredAccess
		win32_share_mode, // dwShareMode
		NULL, // lpSecurityAttributes
		win32_creation_disposition, // dwCreationDisposition
		win32_flags_and_attributes, // dwFlagsAndAttributes
		NULL // hTemplateFile
	);

	if (win32_handle == INVALID_HANDLE_VALUE)
	{
		const DWORD win32_last_error = GetLastError();

		switch (win32_last_error)
		{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				error_code = ec_file_not_found;
				break;

			case ERROR_ACCESS_DENIED:
				error_code = ec_file_access_denied;
				break;

			default:
				error_code = ec_file_open;
				break;
		}

		return false;
	}

	return true;
}

bool file_win32_createfilew_stack(
	const char* path,
	int path_length_with_nul,
	int win32_path_length_with_nul,
	DWORD win32_desired_access,
	DWORD win32_share_mode,
	DWORD win32_creation_disposition,
	DWORD win32_flags_and_attributes,
	HANDLE& win32_handle,
	FileErrorCode& error_code)
{
	WCHAR win32_path[file_win32_stack_path_max_length_with_nul];

	if (MultiByteToWideChar(
		CP_UTF8,
		0,
		path,
		path_length_with_nul,
		win32_path,
		win32_path_length_with_nul) != win32_path_length_with_nul)
	{
		error_code = ec_file_utf8_to_native;
		return false;
	}

	return file_win32_createfilew(
		win32_path,
		win32_desired_access,
		win32_share_mode,
		win32_creation_disposition,
		win32_flags_and_attributes,
		win32_handle,
		error_code);
}

bool file_win32_createfilew_heap(
	const char* path,
	int path_length_with_nul,
	int win32_path_length_with_nul,
	DWORD win32_desired_access,
	DWORD win32_share_mode,
	DWORD win32_creation_disposition,
	DWORD win32_flags_and_attributes,
	WCHAR*& win32_path,
	HANDLE& win32_handle,
	FileErrorCode& error_code)
{
	const int win32_path_byte_count = win32_path_length_with_nul * 2;
	win32_path = static_cast<WCHAR*>(malloc(win32_path_byte_count));

	if (win32_path == NULL)
	{
		error_code = ec_file_out_of_memory;
		return false;
	}

	if (MultiByteToWideChar(
		CP_UTF8,
		0,
		path,
		path_length_with_nul,
		win32_path,
		win32_path_length_with_nul) != win32_path_length_with_nul)
	{
		error_code = ec_file_utf8_to_native;
		return false;
	}

	return file_win32_createfilew(
		win32_path,
		win32_desired_access,
		win32_share_mode,
		win32_creation_disposition,
		win32_flags_and_attributes,
		win32_handle,
		error_code);
}

bool file_win32_open(
	const char* path,
	FileFlags file_flags,
	WCHAR*& win32_path,
	HANDLE& win32_handle,
	FileErrorCode& error_code)
{
	// Validate flags.
	//
	const bool is_create = (file_flags & file_flags_create) != 0;
	const bool is_truncate = (file_flags & file_flags_truncate) != 0;
	const bool is_shared = (file_flags & file_flags_shared) != 0;
	const bool is_exclusive = (file_flags & file_flags_exclusive) != 0;
	const bool is_readable = is_shared || (file_flags & file_flags_read) != 0;
	const bool is_writable = is_create || is_truncate || is_exclusive || (file_flags & file_flags_write) != 0;

	if (!is_readable && !is_writable)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}

	if (is_shared && is_exclusive)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}

	// Desired access.
	const DWORD win32_desired_access = (is_readable ? GENERIC_READ : 0) | (is_writable ? GENERIC_WRITE : 0);

	// Sharing mode.
	const DWORD win32_share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

	// Disposition.
	const DWORD win32_creation_disposition = is_create ? OPEN_ALWAYS : OPEN_EXISTING;

	// Flags and attributes.
	const DWORD win32_flags_and_attributes = FILE_ATTRIBUTE_NORMAL;

	// Calculate UTF-8 path length.
	//
	int path_length = 0;

	for (int i = 0; i < INT_MAX && path[i] != '\0'; ++i)
	{
		++path_length;
	}

	if (path_length == 0 || path_length == INT_MAX)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}

	const int path_length_with_nul = path_length + 1;
	SetLastError(NO_ERROR);
	const int win32_path_length_with_nul = MultiByteToWideChar(
		CP_UTF8, MB_ERR_INVALID_CHARS, path, path_length_with_nul, NULL, 0);
	const DWORD win32_last_error = GetLastError();

	// TODO Remove "last error" check for Windows Vista+.
	if (win32_path_length_with_nul == 0 || win32_last_error != NO_ERROR)
	{
		error_code = ec_file_utf8_to_native;
		return false;
	}

	if (INT_MAX - win32_path_length_with_nul < win32_path_length_with_nul)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}

	if (win32_path_length_with_nul <= file_win32_stack_path_max_length_with_nul)
	{
		if (!file_win32_createfilew_stack(
			path,
			path_length_with_nul,
			win32_path_length_with_nul,
			win32_desired_access,
			win32_share_mode,
			win32_creation_disposition,
			win32_flags_and_attributes,
			win32_handle,
			error_code))
		{
			return false;
		}
	}
	else
	{
		if (!file_win32_createfilew_heap(
			path,
			path_length_with_nul,
			win32_path_length_with_nul,
			win32_desired_access,
			win32_share_mode,
			win32_creation_disposition,
			win32_flags_and_attributes,
			win32_path,
			win32_handle,
			error_code))
		{
			return false;
		}
	}

	// Validate the type.
	//
	const DWORD win32_file_type = GetFileType(win32_handle);

	if (win32_file_type != FILE_TYPE_DISK)
	{
		error_code = ec_file_not_regular;
		return false;
	}

	// Lock if necessary.
	if (is_shared || is_exclusive)
	{
		const DWORD win32_lock_flags = LOCKFILE_FAIL_IMMEDIATELY | (is_exclusive ? LOCKFILE_EXCLUSIVE_LOCK : 0);

		if (!file_win32_lockfileex(win32_handle, win32_lock_flags))
		{
			error_code = ec_file_lock;
			return false;
		}
	}

	// Truncate if necessary.
	//
	if (is_truncate)
	{
		if (!SetEndOfFile(win32_handle))
		{
			error_code = ec_file_truncate;
			return false;
		}
	}

	return true;
}

int file_win32_open(const char* path, FileFlags file_flags, FileErrorCode& error_code)
{
	WCHAR* win32_path = NULL;
	HANDLE win32_handle = INVALID_HANDLE_VALUE;
	const bool result = file_win32_open(path, file_flags, win32_path, win32_handle, error_code);
	free(win32_path);

	if (!result)
	{
		if (win32_handle != INVALID_HANDLE_VALUE)
		{
			file_win32_closehandle(win32_handle);
		}

		return File::invalid_handle;
	}

	return file_win32_cast_handle_to_int(win32_handle);
}

int file_win32_open(const char* path, FileFlags file_flags)
{
	FileErrorCode error_code;
	return file_win32_open(path, file_flags, error_code);
}

int64_t file_win32_setfilepointerex(HANDLE win32_handle, int64_t offset, DWORD win32_origin)
{
	LARGE_INTEGER win32_offset;
	win32_offset.QuadPart = offset;

	LARGE_INTEGER win32_position;

	if (!SetFilePointerEx(win32_handle, win32_offset, &win32_position, win32_origin))
	{
		return -1;
	}

	return win32_position.QuadPart;
}

int64_t file_win32_setfilepointerex(int handle, int64_t offset, DWORD win32_origin)
{
	return file_win32_setfilepointerex(file_win32_cast_int_to_handle(handle), offset, win32_origin);
}

} // namespace

// --------------------------------------------------------------------------

File::File()
	:
	handle_(File::invalid_handle)
{}

File::File(const char* path, FileFlags flags)
	:
	handle_(file_win32_open(path, flags))
{}

File::File(const char* path, FileFlags flags, FileErrorCode& error_code)
	:
	handle_(file_win32_open(path, flags, error_code))
{}

File::~File()
{
	if (!is_open())
	{
		return;
	}

	file_win32_closehandle(handle_);
}

bool File::is_open() const
{
	return handle_ != File::invalid_handle;
}

bool File::open(const char* path, FileFlags file_flags)
{
	FileErrorCode error_code;
	return open(path, file_flags, error_code);
}

bool File::open(const char* path, FileFlags file_flags, FileErrorCode& error_code)
{
	close();
	handle_ = file_win32_open(path, file_flags, error_code);
	return is_open();
}

void File::close()
{
	if (!is_open())
	{
		return;
	}

	file_win32_closehandle(handle_);
	handle_ = File::invalid_handle;
}

intptr_t File::read(void* buffer, intptr_t size) const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);
	const DWORD win32_size = static_cast<DWORD>(std::min(size, file_win32_max_read_write_size));
	DWORD win32_read_size;

	if (!ReadFile(win32_handle, buffer, win32_size, &win32_read_size, NULL))
	{
		return -1;
	}

	return static_cast<intptr_t>(win32_read_size);
}

bool File::read_exactly(void* buffer, intptr_t size) const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);
	unsigned char* dst_bytes = static_cast<unsigned char*>(buffer);

	for (intptr_t dst_index = 0; dst_index < size;)
	{
		const intptr_t rest_size = size - dst_index;
		const DWORD win32_size = static_cast<DWORD>(std::min(rest_size, file_win32_max_read_write_size));
		DWORD win32_read_size;

		if (!ReadFile(win32_handle, &dst_bytes[dst_index], win32_size, &win32_read_size, NULL))
		{
			return false;
		}

		if (win32_read_size == 0)
		{
			return false;
		}

		dst_index += static_cast<int>(win32_read_size);
	}

	return true;
}

intptr_t File::write(const void* buffer, intptr_t size) const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);
	const DWORD win32_size = static_cast<DWORD>(std::min(size, file_win32_max_read_write_size));
	DWORD win32_written_size;

	if (!WriteFile(win32_handle, buffer, win32_size, &win32_written_size, NULL))
	{
		return -1;
	}

	return static_cast<intptr_t>(win32_written_size);
}

bool File::write_exactly(const void* buffer, intptr_t size) const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);
	const unsigned char* src_bytes = static_cast<const unsigned char*>(buffer);

	for (intptr_t src_index = 0; src_index < size;)
	{
		const intptr_t rest_size = size - src_index;
		const DWORD win32_size = static_cast<DWORD>(std::min(rest_size, file_win32_max_read_write_size));
		DWORD win32_written_size;

		if (!WriteFile(win32_handle, &src_bytes[src_index], win32_size, &win32_written_size, NULL))
		{
			return false;
		}

		if (win32_written_size == 0)
		{
			return false;
		}

		src_index += static_cast<int>(win32_written_size);
	}

	return true;
}

int64_t File::seek(int64_t offset, FileOrigin origin) const
{
	DWORD win32_origin;

	switch (origin)
	{
		case file_origin_begin: win32_origin = FILE_BEGIN; break;
		case file_origin_current: win32_origin = FILE_CURRENT; break;
		case file_origin_end: win32_origin = FILE_END; break;
		default: return -1;
	}

	return file_win32_setfilepointerex(handle_, offset, win32_origin);
}

int64_t File::skip(int64_t offset) const
{
	return file_win32_setfilepointerex(handle_, offset, FILE_CURRENT);
}

int64_t File::get_position() const
{
	return skip(0);
}

bool File::set_position(int64_t position) const
{
	return file_win32_setfilepointerex(handle_, position, FILE_BEGIN) >= 0;
}

int64_t File::get_size() const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);
	LARGE_INTEGER win32_size;

	if (!GetFileSizeEx(win32_handle, &win32_size))
	{
		return -1;
	}

	return win32_size.QuadPart;
}

bool File::set_size(int64_t size) const
{
	const HANDLE win32_handle = file_win32_cast_int_to_handle(handle_);

	LARGE_INTEGER position;
	LARGE_INTEGER old_position;

	position.QuadPart = 0;

	if (!SetFilePointerEx(win32_handle, position, &old_position, FILE_CURRENT))
	{
		return false;
	}

	position.QuadPart = size;

	if (!SetFilePointerEx(win32_handle, position, NULL, FILE_BEGIN))
	{
		return false;
	}

	if (!SetEndOfFile(win32_handle))
	{
		return false;
	}

	position = old_position;

	if (!SetFilePointerEx(win32_handle, position, NULL, FILE_BEGIN))
	{
		return false;
	}

	return true;
}

bool File::flush() const
{
	if (!FlushFileBuffers(file_win32_cast_int_to_handle(handle_)))
	{
		return false;
	}

	return true;
}

bool File::lock_shared() const
{
	const DWORD win32_lock_flags = LOCKFILE_FAIL_IMMEDIATELY;
	return file_win32_lockfileex(file_win32_cast_int_to_handle(handle_), win32_lock_flags);
}

bool File::lock_exclusive() const
{
	const DWORD win32_lock_flags = LOCKFILE_FAIL_IMMEDIATELY | LOCKFILE_EXCLUSIVE_LOCK;
	return file_win32_lockfileex(file_win32_cast_int_to_handle(handle_), win32_lock_flags);
}

bool File::unlock() const
{
	return file_win32_unlockfile(file_win32_cast_int_to_handle(handle_));
}

} // namespace bstone

#endif // _WIN32
