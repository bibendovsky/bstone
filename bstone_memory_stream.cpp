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


#include "bstone_memory_stream.h"
#include <cstddef>
#include <algorithm>


namespace bstone {


MemoryStream::MemoryStream(
    int initial_capacity,
    StreamOpenMode open_mode) :
        is_open_(),
        can_read_(),
        can_write_(),
        position_(),
        size_(),
        ext_size_(),
        buffer_(),
        ext_buffer_()
{
    open(initial_capacity, open_mode);
}

MemoryStream::MemoryStream(
    int buffer_size,
    int buffer_offset,
    const uint8_t* buffer,
    StreamOpenMode open_mode) :
        is_open_(),
        can_read_(),
        can_write_(),
        position_(),
        size_(),
        ext_size_(),
        buffer_(),
        ext_buffer_()
{
    open(buffer_size, buffer_offset, buffer, open_mode);
}

// (virtual)
MemoryStream::~MemoryStream()
{
    close();
}

bool MemoryStream::open(
    int initial_capacity,
    StreamOpenMode open_mode)
{
    close();


    bool can_read = false;
    bool can_write = false;

    switch (open_mode) {
    case StreamOpenMode::read:
        can_read = true;
        break;

    case StreamOpenMode::write:
        can_write = true;
        break;

    case StreamOpenMode::read_write:
        can_read = true;
        can_write = true;
        break;

    default:
        return false;
    }


    if (initial_capacity < 0) {
        initial_capacity = 0;
    }

    int_buffer_.reserve(initial_capacity);

    is_open_ = true;
    can_read_ = can_read;
    can_write_ = can_write;

    return true;
}

bool MemoryStream::open(
    int buffer_size,
    int buffer_offset,
    const uint8_t* buffer,
    StreamOpenMode open_mode)
{
    close();

    if (buffer_size < 0) {
        return false;
    }

    if (!buffer) {
        return false;
    }


    bool can_read = false;
    bool can_write = false;

    switch (open_mode) {
    case StreamOpenMode::read:
        can_read = true;
        break;

    case StreamOpenMode::write:
        can_write = true;
        break;

    case StreamOpenMode::read_write:
        can_read = true;
        can_write = true;
        break;

    default:
        return false;
    }


    is_open_ = true;
    can_read_ = can_read;
    can_write_ = can_write;
    size_ = buffer_size;
    ext_size_ = buffer_size;
    buffer_ = const_cast<uint8_t*>(&buffer[buffer_offset]);
    ext_buffer_ = buffer_;

    return true;
}

// (virtual)
void MemoryStream::close()
{
    can_read_ = false;
    can_write_ = false;
    position_ = 0;
    size_ = 0;
    ext_size_ = 0;
    buffer_ = nullptr;
    ext_buffer_ = nullptr;
    Buffer().swap(int_buffer_);
}

// (virtual)
bool MemoryStream::is_open() const
{
    return is_open_;
}

// (virtual)
int64_t MemoryStream::get_size()
{
    return size_;
}

// (virtual)
bool MemoryStream::set_size(
    int64_t size)
{
    if (!is_open()) {
        return false;
    }

    if (!can_write()) {
        return false;
    }

    if (size < 0) {
        return false;
    }

    if (ext_buffer_) {
        return false;
    }

    int_buffer_.resize(static_cast<size_t>(size));

    size_ = size;

    if (size_ > 0) {
        buffer_ = reinterpret_cast<uint8_t*>(&int_buffer_[0]);
    } else {
        buffer_ = nullptr;
    }

    return true;
}

// (virtual)
bool MemoryStream::flush()
{
    return is_open();
}

// (virtual)
int64_t MemoryStream::seek(
    int64_t offset,
    StreamSeekOrigin origin)
{
    if (!is_open()) {
        return -1;
    }

    if (!can_seek()) {
        return -1;
    }

    switch (origin) {
    case StreamSeekOrigin::begin:
        position_ = offset;
        break;

    case StreamSeekOrigin::current:
        position_ += offset;
        break;

    case StreamSeekOrigin::end:
        position_ = size_ + offset;
        break;

    default:
        return -1;
    }

    if (position_ < 0) {
        position_ = 0;
    }

    return position_;
}

// (virtual)
int64_t MemoryStream::get_position()
{
    return position_;
}

// (virtual)
int MemoryStream::read(
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

    auto remain = size_ - position_;

    if (remain <= 0) {
        return 0;
    }

    auto read_count = static_cast<int>(std::min(
        static_cast<int64_t>(count),
        remain));

    std::uninitialized_copy_n(
        &buffer_[position_],
        read_count,
        static_cast<uint8_t*>(buffer));

    position_ += read_count;

    return read_count;
}

// (virtual)
bool MemoryStream::write(
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

    if (!ext_buffer_) {
        auto new_size = position_ + count;

        if (new_size > size_) {
            int_buffer_.resize(static_cast<size_t>(new_size));

            size_ = new_size;
            buffer_ = reinterpret_cast<uint8_t*>(&int_buffer_[0]);
        }
    } else {
        if ((position_ + count) > ext_size_) {
            return false;
        }
    }

    std::uninitialized_copy_n(
        static_cast<const uint8_t*>(buffer),
        count,
        &buffer_[position_]);

    position_ += count;

    return true;
}

// (virtual)
bool MemoryStream::can_read() const
{
    return is_open() && can_read_;
}

// (virtual)
bool MemoryStream::can_seek() const
{
    return is_open();
}

// (virtual)
bool MemoryStream::can_write() const
{
    return is_open() && can_write_;
}

uint8_t* MemoryStream::get_data()
{
    return buffer_;
}

const uint8_t* MemoryStream::get_data() const
{
    return buffer_;
}

bool MemoryStream::remove_block(
    int64_t offset,
    int count)
{
    if (!is_open()) {
        return false;
    }

    if (offset < 0) {
        return false;
    }

    if (count < 0) {
        return false;
    }

    if (count == 0) {
        return true;
    }

    if ((offset + count) > size_) {
        return false;
    }

    int_buffer_.erase(
        int_buffer_.begin() + static_cast<ptrdiff_t>(offset),
        int_buffer_.begin() + static_cast<ptrdiff_t>(offset) + count);

    size_ -= count;

    return true;
}


} // bstone
