/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (POSIX).

#ifndef _WIN32

#include <limits.h>
#include <stdio.h>
#include <algorithm>
#include <sys/file.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "bstone_assert.h"
#include "bstone_file.h"

// ==========================================================================

namespace bstone {

namespace {

typedef struct flock FilePosixFLock;
typedef struct stat FilePosixStat;

template<bool TIs64bit>
struct FilePosixMaxOffT
{
	static const int64_t value = INT32_MAX;
};

template<>
struct FilePosixMaxOffT<true>
{
	static const int64_t value = INT64_MAX;
};

const intptr_t file_posix_max_read_write_size = INTPTR_MAX;
const bool file_posix_is_off_t_64_bit = CHAR_BIT * sizeof(off_t) == 64;

const int64_t file_posix_max_off_t = FilePosixMaxOffT<file_posix_is_off_t_64_bit>::value;

template<typename T>
void file_posix_ignore_result(const T&) {}

// --------------------------------------------------------------------------

bool file_posix_is_off_t_valid(off_t value)
{
	if (file_posix_is_off_t_64_bit)
	{
		return true;
	}

	return std::abs(value) <= file_posix_max_off_t;
}

void file_posix_close(int handle)
{
	const int posix_result = close(handle);
	BSTONE_ASSERT(posix_result == 0);
	file_posix_ignore_result(posix_result);
}

bool file_posix_truncate(int handle, off_t size)
{
	if (!file_posix_is_off_t_valid(size))
	{
		return false;
	}

	return ftruncate(handle, size) == 0;
}

int64_t file_posix_lseek(int handle, int64_t offset, int whence)
{
	if (!file_posix_is_off_t_valid(offset))
	{
		return -1;
	}

	return lseek(handle, offset, whence);
}

bool file_posix_lock_fcntl(int handle, int fcntl_op, short lock_type)
{
	FilePosixFLock posix_flock = FilePosixFLock();
	posix_flock.l_type = lock_type;
	posix_flock.l_whence = SEEK_SET;
	return fcntl(handle, fcntl_op, &posix_flock) == 0;
}

#ifndef F_OFD_SETLK
bool file_posix_lock_flock(int handle, short lock_type)
{
	return flock(handle, lock_type | LOCK_NB) == 0;
}
#endif // F_OFD_SETLK

bool file_posix_lock_shared(int handle)
{
#ifdef F_OFD_SETLK
	return file_posix_lock_fcntl(handle, F_OFD_SETLK, F_RDLCK);
#else
	return file_posix_lock_flock(handle, LOCK_SH);
#endif
}

bool file_posix_lock_exclusive(int handle)
{
#ifdef F_OFD_SETLK
	return file_posix_lock_fcntl(handle, F_OFD_SETLK, F_WRLCK);
#else
	return file_posix_lock_flock(handle, LOCK_EX);
#endif
}

bool file_posix_unlock(int handle)
{
#ifdef F_OFD_SETLK
	return file_posix_lock_fcntl(handle, F_OFD_SETLK, F_UNLCK);
#else
	return file_posix_lock_flock(handle, LOCK_UN);
#endif
}

bool file_posix_open(const char* path, FileFlags flags, int& handle, FileErrorCode& error_code)
{
	const bool is_create = (flags & file_flags_create) != 0;
	const bool is_truncate = (flags & file_flags_truncate) != 0;
	const bool is_shared = (flags & file_flags_shared) != 0;
	const bool is_exclusive = (flags & file_flags_exclusive) != 0;
	const bool is_readable = is_shared || (flags & file_flags_read) != 0;
	const bool is_writable = is_create || is_truncate || is_exclusive || (flags & file_flags_write) != 0;

	if (!is_readable && !is_writable)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}

	// Make status flags, access mode and access permission bits.
	//
	int oflag = 0;
	mode_t mode = 0;

	if (is_create)
	{
		oflag |= O_CREAT;
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
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

	// Open the file.
	//
	handle = ::open(path, oflag, mode);

	if (handle == -1)
	{
		switch (errno)
		{
			case ENOENT: error_code = ec_file_not_found; return false;
			case EACCES: error_code = ec_file_access_denied; return false;
			default: error_code = ec_file_open; return false;
		}
	}

	// Validate a type.
	//
	FilePosixStat posix_stat;

	if (fstat(handle, &posix_stat) != 0)
	{
		error_code = ec_file_open;
		return false;
	}

	if (!S_ISREG(posix_stat.st_mode))
	{
		error_code = ec_file_not_regular;
		return false;
	}

	// Lock the file if necessary.
	//
	if (is_shared)
	{
		if (!file_posix_lock_shared(handle))
		{
			error_code = ec_file_lock;
			return false;
		}
	}
	else if (is_exclusive)
	{
		if (!file_posix_lock_exclusive(handle))
		{
			error_code = ec_file_lock;
			return false;
		}
	}
	else
	{}

	// Truncate the file if necessary
	if (is_truncate)
	{
		if (!file_posix_truncate(handle, 0))
		{
			error_code = ec_file_truncate;
			return false;
		}
	}

	return true;
}

int file_posix_open(const char* path, FileFlags flags, FileErrorCode& error_code)
{
	int handle = File::invalid_handle;

	if (!file_posix_open(path, flags, handle, error_code))
	{
		if (handle != File::invalid_handle)
		{
			file_posix_close(handle);
			handle = File::invalid_handle;
		}
	}

	return handle;
}

int file_posix_open(const char* path, FileFlags flags)
{
	FileErrorCode error_code;
	return file_posix_open(path, flags, error_code);
}

} // namespace

// --------------------------------------------------------------------------

File::File()
	:
	handle_(File::invalid_handle)
{}

File::File(const char* path, FileFlags flags)
	:
	handle_(file_posix_open(path, flags))
{}

File::File(const char* path, FileFlags flags, FileErrorCode& error_code)
	:
	handle_(file_posix_open(path, flags, error_code))
{}

File::~File()
{
	if (!is_open())
	{
		return;
	}

	file_posix_close(handle_);
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
	handle_ = file_posix_open(path, file_flags, error_code);
	return is_open();
}

void File::close()
{
	if (!is_open())
	{
		return;
	}

	file_posix_close(handle_);
	handle_ = File::invalid_handle;
}

intptr_t File::read(void* buffer, intptr_t size) const
{
	return ::read(handle_, buffer, static_cast<size_t>(size));
}

bool File::read_exactly(void* buffer, intptr_t size) const
{
	unsigned char* dst_bytes = static_cast<unsigned char*>(buffer);

	for (intptr_t dst_index = 0; dst_index < size;)
	{
		const intptr_t rest_size = size - dst_index;
		const intptr_t to_read_size = std::min(rest_size, file_posix_max_read_write_size);
		const intptr_t posix_read_size = ::read(handle_, &dst_bytes[dst_index], static_cast<size_t>(to_read_size));

		if (posix_read_size <= 0)
		{
			return false;
		}

		dst_index += posix_read_size;
	}

	return true;
}

intptr_t File::write(const void* buffer, intptr_t size) const
{
	return ::write(handle_, buffer, size);
}

bool File::write_exactly(const void* buffer, intptr_t size) const
{
	const unsigned char* src_bytes = static_cast<const unsigned char*>(buffer);

	for (intptr_t src_index = 0; src_index < size;)
	{
		const intptr_t rest_size = size - src_index;
		const intptr_t to_write_size = std::min(rest_size, file_posix_max_read_write_size);
		const intptr_t posix_written_size = ::write(handle_, &src_bytes[src_index], static_cast<size_t>(to_write_size));

		if (posix_written_size <= 0)
		{
			return false;
		}

		src_index += posix_written_size;
	}

	return true;
}

int64_t File::seek(int64_t offset, FileOrigin origin) const
{
	int posix_origin;

	switch (origin)
	{
		case file_origin_begin: posix_origin = SEEK_SET; break;
		case file_origin_current: posix_origin = SEEK_CUR; break;
		case file_origin_end: posix_origin = SEEK_END; break;
		default: return -1;
	}

	return file_posix_lseek(handle_, offset, posix_origin);
}

int64_t File::skip(int64_t offset) const
{
	return file_posix_lseek(handle_, offset, SEEK_CUR);
}

int64_t File::get_position() const
{
	return skip(0);
}

bool File::set_position(int64_t position) const
{
	return file_posix_lseek(handle_, position, SEEK_SET) >= 0;
}

int64_t File::get_size() const
{
	FilePosixStat posix_stat;
	const int posix_result = fstat(handle_, &posix_stat);

	if (posix_result != 0)
	{
		return -1;
	}

	return posix_stat.st_size;
}

bool File::set_size(int64_t size) const
{
	if (!file_posix_is_off_t_valid(size))
	{
		return false;
	}

	return ftruncate(handle_, static_cast<off_t>(size)) == 0;
}

bool File::flush() const
{
	return fsync(handle_) == 0;
}

bool File::lock_shared() const
{
	return file_posix_lock_shared(handle_);
}

bool File::lock_exclusive() const
{
	return file_posix_lock_exclusive(handle_);
}

bool File::unlock() const
{
	return file_posix_unlock(handle_);
}

} // namespace bstone

#endif // _WIN32
