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

#include "SDL_rwops.h"


namespace bstone
{


namespace
{


SDL_RWops* get_sdl_context(
	void* context)
{
	return static_cast<SDL_RWops*>(context);
}


} // namespace


FileStream::FileStream()
	:
	context_{},
	is_readable_{},
	is_seekable_{},
	is_writable_{}
{
}

FileStream::FileStream(
	const std::string& file_name,
	StreamOpenMode open_mode)
	:
	context_{},
	is_readable_{},
	is_seekable_{},
	is_writable_{}
{
	static_cast<void>(open(file_name, open_mode));
}

FileStream::FileStream(
	FileStream&& rhs)
	:
	context_{std::move(rhs.context_)},
	is_readable_{std::move(rhs.is_readable_)},
	is_seekable_{std::move(rhs.is_seekable_)},
	is_writable_{std::move(rhs.is_writable_)}
{
	rhs.context_ = nullptr;
	rhs.is_readable_ = false;
	rhs.is_seekable_ = false;
	rhs.is_writable_ = false;
}

FileStream::~FileStream()
{
	close_internal();
}

bool FileStream::open(
	const std::string& file_name,
	const StreamOpenMode open_mode)
{
	close_internal();


	auto mode = "";
	bool is_readable = false;
	bool is_writable = false;

	switch (open_mode)
	{
	case StreamOpenMode::read:
		mode = "rb";
		is_readable = true;
		break;

	case StreamOpenMode::write:
		mode = "wb";
		is_writable = true;
		break;

	case StreamOpenMode::read_write:
		mode = "r+b";
		is_readable = true;
		is_writable = true;
		break;

	default:
		return false;
	}


	auto sdl_context = SDL_RWFromFile(file_name.c_str(), mode);

	if (!sdl_context)
	{
		return false;
	}

	context_ = sdl_context;
	is_readable_ = is_readable;
	is_seekable_ = true;
	is_writable_ = is_writable;

	return true;
}

void FileStream::close()
{
	close_internal();
}

bool FileStream::is_open() const
{
	return context_ != nullptr;
}

std::int64_t FileStream::get_size()
{
	if (context_ == nullptr)
	{
		return 0;
	}

	return SDL_RWsize(get_sdl_context(context_));
}

bool FileStream::set_size(
	const std::int64_t size)
{
	static_cast<void>(size);

	return false;
}

std::int64_t FileStream::seek(
	const std::int64_t offset,
	const StreamSeekOrigin origin)
{
	if (context_ == nullptr)
	{
		return -1;
	}

	if (!is_seekable_)
	{
		return -1;
	}


	auto whence = 0;

	switch (origin)
	{
	case StreamSeekOrigin::begin:
		whence = RW_SEEK_SET;
		break;

	case StreamSeekOrigin::current:
		whence = RW_SEEK_CUR;
		break;

	case StreamSeekOrigin::end:
		whence = RW_SEEK_END;
		break;

	default:
		return -1;
	}

	const auto position = SDL_RWseek(get_sdl_context(context_), offset, whence);

	return position;
}

int FileStream::read(
	void* buffer,
	const int count)
{
	if (context_ == nullptr)
	{
		return 0;
	}

	if (!is_readable_)
	{
		return 0;
	}

	if (!buffer)
	{
		return 0;
	}

	if (count <= 0)
	{
		return 0;
	}

	const auto read_result = static_cast<int>(SDL_RWread(get_sdl_context(context_), buffer, 1, count));

	return read_result;
}

bool FileStream::write(
	const void* buffer,
	const int count)
{
	if (context_ == nullptr)
	{
		return false;
	}

	if (!is_writable_)
	{
		return false;
	}

	if (count < 0)
	{
		return false;
	}

	if (count == 0)
	{
		return true;
	}

	if (!buffer)
	{
		return false;
	}

	auto write_result = static_cast<int>(SDL_RWwrite(get_sdl_context(context_), buffer, 1, count));

	return write_result == count;
}

bool FileStream::flush()
{
	return context_ != nullptr;
}

bool FileStream::is_readable() const
{
	return context_ != nullptr && is_readable_;
}

bool FileStream::is_seekable() const
{
	return context_ != nullptr && is_seekable_;
}

bool FileStream::is_writable() const
{
	return context_ != nullptr && is_writable_;
}

bool FileStream::is_exists(
	const std::string& file_name)
{
	const auto is_open = FileStream{file_name}.is_open();

	return is_open;
}

void FileStream::close_internal()
{
	if (context_ != nullptr)
	{
		auto sdl_context = get_sdl_context(context_);
		static_cast<void>(SDL_RWclose(sdl_context));
		context_ = nullptr;
	}

	is_readable_ = false;
	is_seekable_ = false;
	is_writable_ = false;
}


} // bstone
