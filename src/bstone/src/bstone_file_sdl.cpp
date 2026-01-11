/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (SDL)

#include "bstone_file.h"
#include <cstddef>
#include "SDL3/SDL_iostream.h"

namespace bstone {

File::File(const char* path, FileFlags flags)
{
	open(path, flags);
}

File::File(const char* path, FileFlags flags, FileErrorCode& error_code)
{
	open(path, flags, error_code);
}

File::~File()
{
	SDL_CloseIO(static_cast<SDL_IOStream*>(handle_));
}

bool File::is_open() const
{
	return handle_ != nullptr;
}

bool File::open(const char* path, FileFlags flags)
{
	FileErrorCode error_code;
	return open(path, flags, error_code);
}

bool File::open(const char* path, FileFlags flags, FileErrorCode& error_code)
{
	close();
	const bool flag_shared = ((flags & file_flags_shared) != 0);
	const bool flag_exclusive = ((flags & file_flags_exclusive) != 0);
	const bool flag_create = ((flags & file_flags_create) != 0);
	const bool flag_truncate = ((flags & file_flags_truncate) != 0);
	const bool flag_read = ((flags & file_flags_read) != 0) || flag_shared;
	const bool flag_write = ((flags & file_flags_write) != 0) || flag_create || flag_truncate || flag_exclusive;
	const char* sdl_mode = nullptr;
	const char* sdl_retry_mode = nullptr;
	if (!(flag_shared && flag_exclusive))
	{
		if (flag_read && !flag_write && !flag_create && !flag_truncate)
		{
			sdl_mode = "rb";
		}
		else if (flag_write)
		{
			if (!flag_create && !flag_truncate)
			{
				sdl_mode = "r+b";
			}
			else if (!flag_create && flag_truncate)
			{}
			else if (flag_create && !flag_truncate)
			{
				sdl_mode = "r+b";
				if (flag_read)
				{
					sdl_retry_mode = "w+xb";
				}
				else
				{
					sdl_retry_mode = "wxb";
				}
			}
			else if (flag_create && flag_truncate)
			{
				if (flag_read)
				{
					sdl_mode = "w+b";
				}
				else
				{
					sdl_mode = "wb";
				}
			}
		}
	}
	if (sdl_mode == nullptr)
	{
		error_code = ec_file_invalid_arguments;
		return false;
	}
	handle_ = SDL_IOFromFile(path, sdl_mode);
	if (handle_ == nullptr && sdl_retry_mode != nullptr)
	{
		handle_ = SDL_IOFromFile(path, sdl_retry_mode);
	}
	if (handle_ == nullptr)
	{
		error_code = ec_file_unknown;
		return false;
	}
	error_code = ec_file_none;
	is_readable_ = flag_read;
	return true;
}

void File::close()
{
	SDL_CloseIO(static_cast<SDL_IOStream*>(handle_));
	handle_ = nullptr;
	is_readable_ = false;
}

std::intptr_t File::read(void* buffer, std::intptr_t size) const
{
	if (!is_readable_)
	{
		return -1;
	}
	SDL_IOStream* const sdl_io_stream = static_cast<SDL_IOStream*>(handle_);
	const std::size_t sdl_read_size = SDL_ReadIO(sdl_io_stream, buffer, static_cast<std::size_t>(size));
	if (sdl_read_size == 0)
	{
		const SDL_IOStatus sdl_io_status = SDL_GetIOStatus(sdl_io_stream);
		if (sdl_io_status != SDL_IO_STATUS_READY && sdl_io_status != SDL_IO_STATUS_EOF)
		{
			return -1;
		}
	}
	return static_cast<std::intptr_t>(sdl_read_size);
}

bool File::read_exactly(void* buffer, std::intptr_t size) const
{
	if (!is_readable_)
	{
		return false;
	}
	SDL_IOStream* const sdl_io_stream = static_cast<SDL_IOStream*>(handle_);
	std::intptr_t offset = 0;
	std::byte* const dst_bytes = static_cast<std::byte*>(buffer);
	while (offset < size)
	{
		const std::size_t sdl_read_size = SDL_ReadIO(
			sdl_io_stream,
			dst_bytes + offset,
			static_cast<std::size_t>(size - offset));
		if (sdl_read_size == 0)
		{
			return false;
		}
		offset += static_cast<std::intptr_t>(sdl_read_size);
	}
	return true;
}

std::intptr_t File::write(const void* buffer, std::intptr_t size) const
{
	SDL_IOStream* const sdl_io_stream = static_cast<SDL_IOStream*>(handle_);
	const std::size_t sdl_written_size = SDL_WriteIO(sdl_io_stream, buffer, static_cast<std::size_t>(size));
	if (sdl_written_size == 0)
	{
		const SDL_IOStatus sdl_io_status = SDL_GetIOStatus(sdl_io_stream);
		if (sdl_io_status != SDL_IO_STATUS_READY)
		{
			return -1;
		}
	}
	return static_cast<std::intptr_t>(sdl_written_size);
}

bool File::write_exactly(const void* buffer, std::intptr_t size) const
{
	SDL_IOStream* const sdl_io_stream = static_cast<SDL_IOStream*>(handle_);
	std::intptr_t offset = 0;
	const std::byte* const src_bytes = static_cast<const std::byte*>(buffer);
	while (offset < size)
	{
		const std::size_t sdl_written_size = SDL_WriteIO(
			sdl_io_stream,
			src_bytes + offset,
			static_cast<std::size_t>(size - offset));
		if (sdl_written_size == 0)
		{
			return false;
		}
		offset += static_cast<std::intptr_t>(sdl_written_size);
	}
	return true;
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	SDL_IOWhence sdl_io_whence;
	switch (origin)
	{
		case file_origin_begin:
			sdl_io_whence = SDL_IO_SEEK_SET;
			break;
		case file_origin_current:
			sdl_io_whence = SDL_IO_SEEK_CUR;
			break;
		case file_origin_end:
			sdl_io_whence = SDL_IO_SEEK_END;
			break;
		default:
			return -1;
	}
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), offset, sdl_io_whence);
}

std::int64_t File::skip(std::int64_t offset) const
{
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), offset, SDL_IO_SEEK_CUR);
}

std::int64_t File::get_position() const
{
	return SDL_TellIO(static_cast<SDL_IOStream*>(handle_));
}

bool File::set_position(std::int64_t position) const
{
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), position, SDL_IO_SEEK_SET) >= 0;
}

std::int64_t File::get_size() const
{
	return SDL_GetIOSize(static_cast<SDL_IOStream*>(handle_));
}

bool File::set_size([[maybe_unused]] std::int64_t size) const
{
	return false;
}

bool File::flush() const
{
	return SDL_FlushIO(static_cast<SDL_IOStream*>(handle_));
}

} // namespace bstone
