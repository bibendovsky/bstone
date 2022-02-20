/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// File stream.
//


#include "bstone_file_stream.h"


#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static_assert(sizeof(::off_t) >= sizeof(std::int64_t), "Unsupported file size type.");
#endif // _WIN32

#include <cassert>

#include <memory>
#include <utility>


namespace bstone
{


namespace
{


const auto file_stream_invalid_handle =
#ifdef _WIN32
	INVALID_HANDLE_VALUE
#else
	-1
#endif // _WIN32
;


#if _WIN32
bool win32_is_utf8() noexcept
{
	static const auto result = (::GetACP() == CP_UTF8);
	return result;
}

int win32_get_utf8_to_utf16_size(
	const char* u8_string,
	int u8_string_size) noexcept
{
	assert(u8_string);
	assert(u8_string_size > 0);

	::WCHAR buffer;

	return ::MultiByteToWideChar(
		CP_UTF8,
		0,
		u8_string,
		u8_string_size,
		&buffer,
		0
	);
}

int win32_utf8_to_utf16(
	const char* u8_string,
	int u8_string_size,
	::LPWSTR u16_string,
	int u16_string_size) noexcept
{
	assert(u8_string);
	assert(u8_string_size > 0);
	assert(u16_string);
	assert(u16_string_size > 0);

	return ::MultiByteToWideChar(
		CP_UTF8,
		0,
		u8_string,
		u8_string_size,
		u16_string,
		u16_string_size
	);
}


::HANDLE win32_create_file_utf16_convert_and_call(
	::LPCSTR lpFileName,
	int lpFileName_size,
	::LPWSTR u16_lpFileName,
	int u16_lpFileName_size,
	::DWORD dwDesiredAccess,
	::DWORD dwShareMode,
	::DWORD dwCreationDisposition)
{
	const auto u16_nul_index = win32_utf8_to_utf16(
		lpFileName,
		lpFileName_size,
		u16_lpFileName,
		u16_lpFileName_size
	);

	u16_lpFileName[u16_nul_index] = L'\0';

	return ::CreateFileW(
		u16_lpFileName,
		dwDesiredAccess,
		dwShareMode,
		nullptr,
		dwCreationDisposition,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);
}

::HANDLE win32_create_file_utf16_static(
	::LPCSTR lpFileName,
	int lpFileName_size,
	int u16_lpFileName_size,
	::DWORD dwDesiredAccess,
	::DWORD dwShareMode,
	::DWORD dwCreationDisposition)
{
	::WCHAR u16_buffer[MAX_PATH + 1];

	return win32_create_file_utf16_convert_and_call(
		lpFileName,
		lpFileName_size,
		u16_buffer,
		u16_lpFileName_size,
		dwDesiredAccess,
		dwShareMode,
		dwCreationDisposition
	);
}

::HANDLE win32_create_file_utf16_dynamic(
	::LPCSTR lpFileName,
	int lpFileName_size,
	int u16_lpFileName_size,
	::DWORD dwDesiredAccess,
	::DWORD dwShareMode,
	::DWORD dwCreationDisposition)
{
	auto u16_buffer = std::make_unique<::WCHAR[]>(u16_lpFileName_size + 1);

	return win32_create_file_utf16_convert_and_call(
		lpFileName,
		lpFileName_size,
		u16_buffer.get(),
		u16_lpFileName_size,
		dwDesiredAccess,
		dwShareMode,
		dwCreationDisposition
	);
}

::HANDLE win32_create_file_utf16(
	::LPCSTR lpFileName,
	int lpFileName_size,
	::DWORD dwDesiredAccess,
	::DWORD dwShareMode,
	::DWORD dwCreationDisposition)
{
	const auto u16_file_name_size = win32_get_utf8_to_utf16_size(lpFileName, lpFileName_size);

	if (u16_file_name_size <= MAX_PATH)
	{
		return win32_create_file_utf16_static(
			lpFileName,
			lpFileName_size,
			u16_file_name_size,
			dwDesiredAccess,
			dwShareMode,
			dwCreationDisposition
		);
	}
	else
	{
		return win32_create_file_utf16_dynamic(
			lpFileName,
			lpFileName_size,
			u16_file_name_size,
			dwDesiredAccess,
			dwShareMode,
			dwCreationDisposition
		);
	}
}


::DWORD win32_get_file_attributes_utf16_convert_and_call(
	::LPCSTR lpFileName,
	int lpFileName_size,
	::LPWSTR u16_lpFileName,
	int u16_lpFileName_size)
{
	const auto u16_nul_index = win32_utf8_to_utf16(
		lpFileName,
		lpFileName_size,
		u16_lpFileName,
		u16_lpFileName_size
	);

	u16_lpFileName[u16_nul_index] = L'\0';

	return ::GetFileAttributesW(u16_lpFileName);
}

::DWORD win32_get_file_attributes_utf16_static(
	::LPCSTR lpFileName,
	int lpFileName_size,
	int u16_lpFileName_size)
{
	::WCHAR u16_buffer[MAX_PATH + 1];

	return win32_get_file_attributes_utf16_convert_and_call(
		lpFileName,
		lpFileName_size,
		u16_buffer,
		u16_lpFileName_size
	);
}

::DWORD win32_get_file_attributes_utf16_dynamic(
	::LPCSTR lpFileName,
	int lpFileName_size,
	int u16_lpFileName_size)
{
	auto u16_buffer = std::make_unique<::WCHAR[]>(u16_lpFileName_size + 1);

	return win32_get_file_attributes_utf16_convert_and_call(
		lpFileName,
		lpFileName_size,
		u16_buffer.get(),
		u16_lpFileName_size
	);
}

::DWORD win32_get_file_attributes_utf16(
	::LPCSTR lpFileName,
	int lpFileName_size)
{
	const auto u16_file_name_size = win32_get_utf8_to_utf16_size(lpFileName, lpFileName_size);

	if (u16_file_name_size <= MAX_PATH)
	{
		return win32_get_file_attributes_utf16_static(
			lpFileName,
			lpFileName_size,
			u16_file_name_size
		);
	}
	else
	{
		return win32_get_file_attributes_utf16_dynamic(
			lpFileName,
			lpFileName_size,
			u16_file_name_size
		);
	}
}
#endif // _WIN32


} // namespace


FileStream::FileStream() noexcept
	:
	handle_{file_stream_invalid_handle}
{
}

FileStream::FileStream(
	const std::string& file_name,
	StreamOpenMode open_mode) noexcept
	:
	FileStream{}
{
	static_cast<void>(open(file_name, open_mode));
}

FileStream::FileStream(
	FileStream&& rhs) noexcept
	:
	FileStream{}
{
	std::swap(handle_, rhs.handle_);
	std::swap(is_readable_, rhs.is_readable_);
	std::swap(is_seekable_, rhs.is_seekable_);
	std::swap(is_writable_, rhs.is_writable_);
}

FileStream::~FileStream()
{
	if (is_open_internal())
	{
		close_handle();
	}
}

bool FileStream::open(
	const std::string& file_name,
	StreamOpenMode open_mode) noexcept
{
	close_internal();

	if (file_name.empty())
	{
		return false;
	}

#ifdef _WIN32
	auto is_readable = false;
	auto is_writable = false;
	auto is_accept_already_exists = false;

	auto win32_desired_access = ::DWORD{};
	constexpr auto win32_share_mode = ::DWORD{FILE_SHARE_READ};
	auto win32_creation_disposition = ::DWORD{};

	switch (open_mode)
	{
		case StreamOpenMode::read:
			is_readable = true;
			win32_desired_access = GENERIC_READ;
			win32_creation_disposition = OPEN_EXISTING;
			break;

		case StreamOpenMode::write:
			is_writable = true;
			is_accept_already_exists = true;
			win32_desired_access = GENERIC_WRITE;
			win32_creation_disposition = CREATE_ALWAYS;
			break;

		case StreamOpenMode::read_write:
			is_readable = true;
			is_writable = true;
			is_accept_already_exists = true;
			win32_desired_access = GENERIC_READ | GENERIC_WRITE;
			win32_creation_disposition = OPEN_ALWAYS;
			break;

		default:
			return false;
	}

	auto win32_handle = ::HANDLE{};

	if (win32_is_utf8())
	{
		win32_handle = ::CreateFileA(
			file_name.c_str(),
			win32_desired_access,
			win32_share_mode,
			nullptr,
			win32_creation_disposition,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
	}
	else
	{
		win32_handle = win32_create_file_utf16(
			file_name.c_str(),
			static_cast<int>(file_name.size()),
			win32_desired_access,
			win32_share_mode,
			win32_creation_disposition
		);
	}

	if (win32_handle == INVALID_HANDLE_VALUE)
	{
		const auto win32_last_error = ::GetLastError();

		if (!(is_accept_already_exists &&
			win32_last_error == ERROR_ALREADY_EXISTS))
		{
			return false;
		}
	}

	handle_ = win32_handle;
#else
	auto is_readable = false;
	auto is_writable = false;

	auto posix_oflag = 0;

	switch (open_mode)
	{
		case StreamOpenMode::read:
			is_readable = true;
			posix_oflag = O_RDONLY;
			break;

		case StreamOpenMode::write:
			is_writable = true;
			posix_oflag = O_WRONLY | O_CREAT;
			break;

		case StreamOpenMode::read_write:
			is_readable = true;
			is_writable = true;
			posix_oflag = O_RDWR | O_CREAT;
			break;

		default:
			return false;
	}

	const auto posix_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
	const auto posix_handle = ::open(file_name.c_str(), posix_oflag, posix_mode);

	if (posix_handle == -1)
	{
		return false;
	}

	struct ::stat posix_stat;

	const auto fstat_result = ::fstat(posix_handle, &posix_stat);

	if (fstat_result != 0 || !S_ISREG(posix_stat.st_mode))
	{
		static_cast<void>(::close(posix_handle));

		return false;
	}

	handle_ = posix_handle;
#endif // _WIN32

	is_readable_ = is_readable;
	is_seekable_ = true;
	is_writable_ = is_writable;

	return true;
}

void FileStream::close() noexcept
{
	close_internal();
}

bool FileStream::is_open() const noexcept
{
	return is_open_internal();
}

std::int64_t FileStream::get_size() noexcept
{
	if (!is_open_internal())
	{
		return 0;
	}


#ifdef _WIN32
	::LARGE_INTEGER win32_size;

	const auto win32_result = ::GetFileSizeEx(handle_, &win32_size);

	if (win32_result == 0)
	{
		return 0;
	}

	return win32_size.QuadPart;
#else
	struct ::stat posix_stat;

	const auto fstat_result = ::fstat(handle_, &posix_stat);

	if (fstat_result != 0)
	{
		return 0;
	}

	return posix_stat.st_size;
#endif // _WIN32
}

bool FileStream::set_size(
	std::int64_t size) noexcept
{
	static_cast<void>(size);

	return false;
}

std::int64_t FileStream::seek(
	std::int64_t offset,
	StreamSeekOrigin origin) noexcept
{
	if (!is_open_internal())
	{
		return -1;
	}

	if (!is_seekable_)
	{
		return -1;
	}

#ifdef _WIN32
	auto win32_move_method = ::DWORD{};

	switch (origin)
	{
		case StreamSeekOrigin::begin:
			win32_move_method = FILE_BEGIN;
			break;

		case StreamSeekOrigin::current:
			win32_move_method = FILE_CURRENT;
			break;

		case StreamSeekOrigin::end:
			win32_move_method = FILE_END;
			break;

		default:
			return -1;
	}

	::LARGE_INTEGER win32_distance_to_move;
	win32_distance_to_move.QuadPart = offset;

	::LARGE_INTEGER win32_new_file_pointer;

	const auto win32_result = ::SetFilePointerEx(
		handle_,
		win32_distance_to_move,
		&win32_new_file_pointer,
		win32_move_method
	);

	if (win32_result == 0)
	{
		return -1;
	}

	return win32_new_file_pointer.QuadPart;
#else
	auto posix_whence = 0;

	switch (origin)
	{
		case StreamSeekOrigin::begin:
			posix_whence = SEEK_SET;
			break;

		case StreamSeekOrigin::current:
			posix_whence = SEEK_CUR;
			break;

		case StreamSeekOrigin::end:
			posix_whence = SEEK_END;
			break;

		default:
			return -1;
	}

	const auto posix_result = ::lseek(handle_, offset, posix_whence);

	return posix_result;
#endif // _WIN32
}

int FileStream::read(
	void* buffer,
	int count) noexcept
{
	if (!is_open_internal() ||
		!is_readable_ ||
		!buffer ||
		count < 0)
	{
		return 0;
	}

#ifdef _WIN32
	::DWORD win32_number_of_bytes_read;

	const auto win32_result = ::ReadFile(
		handle_,
		buffer,
		static_cast<::DWORD>(count),
		&win32_number_of_bytes_read,
		nullptr
	);

	if (win32_result == 0)
	{
		return 0;
	}

	return static_cast<int>(win32_number_of_bytes_read);
#else
	const auto posix_result = ::read(handle_, buffer, static_cast<size_t>(count));

	if (posix_result == -1)
	{
		return 0;
	}

	return static_cast<int>(posix_result);
#endif // _WIN32
}

bool FileStream::write(
	const void* buffer,
	int count) noexcept
{
	if (!is_open_internal() ||
		!is_writable_ ||
		count < 0 ||
		!buffer)
	{
		return false;
	}

#ifdef _WIN32
	::DWORD win32_number_of_bytes_written;

	const auto win32_result = ::WriteFile(
		handle_,
		buffer,
		static_cast<::DWORD>(count),
		&win32_number_of_bytes_written,
		nullptr
	);

	if (win32_result == 0)
	{
		return false;
	}

	return win32_number_of_bytes_written == static_cast<DWORD>(count);
#else
	const auto posix_result = ::write(handle_, buffer, static_cast<size_t>(count));

	if (posix_result == -1)
	{
		return false;
	}

	return posix_result == count;
#endif // _WIN32
}

bool FileStream::flush() noexcept
{
	if (!is_open_internal())
	{
		return false;
	}

#ifdef _WIN32
	const auto win32_result = ::FlushFileBuffers(handle_);

	if (win32_result == 0)
	{
		return false;
	}

	return true;
#else
	const auto posix_result = ::fsync(handle_);

	if (posix_result != 0)
	{
		return false;
	}

	return true;
#endif // _WIN32
}

bool FileStream::is_readable() const noexcept
{
	return is_open_internal() && is_readable_;
}

bool FileStream::is_seekable() const noexcept
{
	return is_open_internal() && is_seekable_;
}

bool FileStream::is_writable() const noexcept
{
	return is_open_internal() && is_writable_;
}

bool FileStream::is_exists(
	const std::string& file_name) noexcept
{
	if (file_name.empty())
	{
		return false;
	}

#ifdef _WIN32
	auto win32_file_attributes = ::DWORD{};

	if (win32_is_utf8())
	{
		win32_file_attributes = ::GetFileAttributesA(file_name.c_str());
	}
	else
	{
		win32_file_attributes = win32_get_file_attributes_utf16(
			file_name.c_str(),
			static_cast<int>(file_name.size())
		);
	}

	if (win32_file_attributes == INVALID_FILE_ATTRIBUTES ||
		(win32_file_attributes & FILE_ATTRIBUTE_NORMAL) == 0)
	{
		return false;
	}

	return true;
#else
	struct ::stat posix_stat;

	const auto posix_result = ::stat(file_name.c_str(), &posix_stat);

	if (posix_result != 0 || !S_ISREG(posix_stat.st_mode))
	{
		return false;
	}

	return true;
#endif // _WIN32
}

bool FileStream::is_open_internal() const noexcept
{
	return handle_ != file_stream_invalid_handle;
}

void FileStream::close_handle() noexcept
{
#ifdef _WIN32
	const auto win32_result = ::CloseHandle(static_cast<::HANDLE>(handle_));

#ifdef NDEBUG
	static_cast<void>(win32_result);
#else
	assert(win32_result != 0);
#endif // NDEBUG
#else
	const auto posix_result = ::close(handle_);

#ifdef NDEBUG
	static_cast<void>(posix_result);
#else
	assert(posix_result == 0);
#endif // NDEBUG
#endif // _WIN32
}

void FileStream::close_internal() noexcept
{
	if (!is_open_internal())
	{
		return;
	}

	close_handle();

	handle_ = file_stream_invalid_handle;
	is_readable_ = false;
	is_seekable_ = false;
	is_writable_ = false;
}


} // bstone
