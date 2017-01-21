/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
#include "SDL.h"


namespace bstone {


namespace {


SDL_RWops* get_sdl_context(
    void* context)
{
    return static_cast<SDL_RWops*>(context);
}


} // namespace


FileStream::FileStream() :
        context_(),
        can_read_(),
        can_seek_(),
        can_write_()
{
}

FileStream::FileStream(
    const std::string& file_name,
    StreamOpenMode open_mode) :
        context_(),
        can_read_(),
        can_seek_(),
        can_write_()
{
    open(file_name, open_mode);
}

// (virtual)
FileStream::~FileStream()
{
    close();
}

bool FileStream::open(
    const std::string& file_name,
    StreamOpenMode open_mode)
{
    close();


    auto mode = "";
    bool is_readable = false;
    bool is_writable = false;

    switch (open_mode) {
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


    auto sdl_context = ::SDL_RWFromFile(file_name.c_str(), mode);

    if (!sdl_context) {
        return false;
    }

    context_ = sdl_context;
    can_read_ = is_readable;
    can_seek_ = true;
    can_write_ = is_writable;

    return true;
}

// (virtual)
void FileStream::close()
{
    if (context_) {
        auto sdl_context = get_sdl_context(context_);
        static_cast<void>(SDL_RWclose(sdl_context));
        context_ = nullptr;
    }

    can_read_ = false;
    can_seek_ = false;
    can_write_ = false;
}

// (virtual)
bool FileStream::is_open() const
{
    return context_ != nullptr;
}

// (virtual)
int64_t FileStream::get_size()
{
    if (!is_open()) {
        return 0;
    }

    return SDL_RWsize(get_sdl_context(context_));
}

// (virtual)
bool FileStream::set_size(
    int64_t size)
{
    static_cast<void>(size);
    return false;
}

// (virtual)
int64_t FileStream::seek(
    int64_t offset,
    StreamSeekOrigin origin)
{
    if (!is_open()) {
        return -1;
    }

    if (!can_seek()) {
        return -1;
    }


    int whence = 0;

    switch (origin) {
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

    auto position = SDL_RWseek(
        get_sdl_context(context_),
        offset,
        whence);

    return position;
}

// (virtual)
int FileStream::read(
    void* buffer,
    int count)
{
    if (!is_open()) {
        return 0;
    }

    if (!can_read()) {
        return 0;
    }

    if (!buffer) {
        return 0;
    }

    if (count <= 0) {
        return 0;
    }

    return static_cast<int>(SDL_RWread(
        get_sdl_context(context_),
        buffer,
        1,
        count));
}

// (virtual)
bool FileStream::write(
    const void* buffer,
    int count)
{
    if (!is_open()) {
        return false;
    }

    if (!can_write()) {
        return false;
    }

    if (count < 0) {
        return false;
    }

    if (count == 0) {
        return true;
    }

    if (!buffer) {
        return false;
    }

    auto written_count = static_cast<int>(SDL_RWwrite(
        get_sdl_context(context_),
        buffer,
        1,
        count));

    return written_count == count;
}

// (virtual)
bool FileStream::can_read() const
{
    return is_open() && can_read_;
}

// (virtual)
bool FileStream::can_seek() const
{
    return is_open() && can_seek_;
}

// (virtual)
bool FileStream::can_write() const
{
    return is_open() && can_write_;
}

// (static)
bool FileStream::is_exists(
    const std::string& file_name)
{
    return FileStream(file_name).is_open();
}


} // bstone
