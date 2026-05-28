/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File primitive (SDL)

#include "bstone_sys_file.h"
#include "bstone_assert.h"
#include <cstddef>
#include <memory>
#include "SDL3/SDL_iostream.h"

namespace bstone::sys {

File::File(const char* path, FileMode mode)
{
	open(path, mode);
}

File::File(File&& rhs) noexcept
	:
	handle_{rhs.handle_}
{
	rhs.handle_ = nullptr;
}

File& File::operator=(File&& rhs) noexcept
{
	close();
	handle_ = rhs.handle_;
	rhs.handle_ = nullptr;
	return *this;
}

File::~File()
{
	SDL_CloseIO(static_cast<SDL_IOStream*>(handle_));
}

bool File::is_open() const
{
	return handle_ != nullptr;
}

bool File::open(const char* path, FileMode mode)
{
	close();
	const char* sdl_mode;
	switch (mode)
	{
		case FileMode::read:
			sdl_mode = "rb";
			break;
		case FileMode::create:
			sdl_mode = "wb";
			break;
		default:
			BSTONE_ASSERT(false && "Unknown mode.");
			return false;
	}
	handle_ = SDL_IOFromFile(path, sdl_mode);
	return is_open();
}

void File::close()
{
	SDL_CloseIO(static_cast<SDL_IOStream*>(handle_));
	handle_ = nullptr;
}

int File::read(void* buffer, int size) const
{
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(size >= 0);
	const std::size_t sdl_read_size = SDL_ReadIO(
		static_cast<SDL_IOStream*>(handle_),
		buffer,
		static_cast<std::size_t>(size));
	if (sdl_read_size == 0)
	{
		const SDL_IOStatus sdl_io_status = SDL_GetIOStatus(static_cast<SDL_IOStream*>(handle_));
		if (!(sdl_io_status == SDL_IO_STATUS_READY || sdl_io_status == SDL_IO_STATUS_EOF))
			return -1;
	}
	return static_cast<int>(sdl_read_size);
}

bool File::read_exactly(void* buffer, int size) const
{
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(size >= 0);
	for (int offset = 0; offset < size;)
	{
		const std::size_t sdl_read_size = SDL_ReadIO(
			static_cast<SDL_IOStream*>(handle_),
			static_cast<std::byte*>(buffer) + offset,
			static_cast<std::size_t>(size - offset));
		if (sdl_read_size == 0)
			return false;
		offset += static_cast<int>(sdl_read_size);
	}
	return true;
}

int File::write(const void* buffer, int size) const
{
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(size >= 0);
	const std::size_t sdl_written_size = SDL_WriteIO(
		static_cast<SDL_IOStream*>(handle_),
		buffer,
		static_cast<std::size_t>(size));
	if (sdl_written_size == 0)
	{
		const SDL_IOStatus sdl_io_status = SDL_GetIOStatus(static_cast<SDL_IOStream*>(handle_));
		if (sdl_io_status != SDL_IO_STATUS_READY)
			return -1;
	}
	return static_cast<int>(sdl_written_size);
}

bool File::write_exactly(const void* buffer, int size) const
{
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(size >= 0);
	for (int offset = 0; offset < size;)
	{
		const std::size_t sdl_written_size = SDL_WriteIO(
			static_cast<SDL_IOStream*>(handle_),
			static_cast<const std::byte*>(buffer) + offset,
			static_cast<std::size_t>(size - offset));
		if (sdl_written_size == 0)
			return false;
		offset += static_cast<int>(sdl_written_size);
	}
	return true;
}

std::int64_t File::seek(std::int64_t offset, FileOrigin origin) const
{
	BSTONE_ASSERT(is_open());
	SDL_IOWhence sdl_io_whence;
	switch (origin)
	{
		case FileOrigin::begin:
			sdl_io_whence = SDL_IO_SEEK_SET;
			break;
		case FileOrigin::current:
			sdl_io_whence = SDL_IO_SEEK_CUR;
			break;
		case FileOrigin::end:
			sdl_io_whence = SDL_IO_SEEK_END;
			break;
		default:
			BSTONE_ASSERT(false && "Unknown origin.");
			return -1;
	}
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), offset, sdl_io_whence);
}

std::int64_t File::skip(std::int64_t offset) const
{
	BSTONE_ASSERT(is_open());
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), offset, SDL_IO_SEEK_CUR);
}

std::int64_t File::get_position() const
{
	BSTONE_ASSERT(is_open());
	return SDL_TellIO(static_cast<SDL_IOStream*>(handle_));
}

bool File::set_position(std::int64_t position) const
{
	BSTONE_ASSERT(is_open());
	BSTONE_ASSERT(position >= 0);
	return SDL_SeekIO(static_cast<SDL_IOStream*>(handle_), position, SDL_IO_SEEK_SET) >= 0;
}

std::int64_t File::get_size() const
{
	BSTONE_ASSERT(is_open());
	return SDL_GetIOSize(static_cast<SDL_IOStream*>(handle_));
}

bool File::flush() const
{
	BSTONE_ASSERT(is_open());
	return SDL_FlushIO(static_cast<SDL_IOStream*>(handle_));
}

} // namespace bstone::sys
