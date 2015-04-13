/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//
// File stream.
//


#include "bstone_file_stream.h"


namespace bstone {


FileStream::FileStream() :
        size_(),
        position_(),
        can_read_(),
        can_seek_(),
        can_write_(),
        need_sync_read_position_(),
        need_sync_write_position_()
{
}

FileStream::FileStream(
    const std::string& file_name,
    StreamOpenMode open_mode) :
        size_(),
        position_(),
        can_read_(),
        can_seek_(),
        can_write_(),
        need_sync_read_position_(),
        need_sync_write_position_()
{
    open(file_name, open_mode);
}

// (virtual)
FileStream::~FileStream()
{
}

bool FileStream::open(
    const std::string& file_name,
    StreamOpenMode open_mode)
{
    close();

    if ((open_mode & STREAM_OPEN_READ_WRITE) == 0) {
        return false;
    }

    auto can_read = false;
    auto can_seek = false;
    auto can_write = false;

    auto mode = std::ios::binary | std::ios::ate;

    if ((open_mode & STREAM_OPEN_READ) != 0) {
        can_read = true;
        mode |= std::ios::in;
    }

    if ((open_mode & STREAM_OPEN_WRITE) != 0) {
        can_write = true;
        mode |= std::ios::out;
    }

    if ((open_mode & STREAM_OPEN_TRUNCATE) != 0) {
        if (!can_write) {
            return false;
        }

        mode |= std::ios::trunc;
    }

    stream_.open(file_name.c_str(), mode);

    if (!stream_.is_open()) {
        return false;
    }

    auto is_succeed = true;

    std::ios::pos_type end_pos = -1;

    if (is_succeed) {
        if (can_read) {
            end_pos = stream_.tellg();
        } else {
            end_pos = stream_.tellp();
        }

        stream_.clear();
    }

    if (is_succeed) {
        if (can_read) {
            stream_.seekg(0);
            std::ios::pos_type pos = stream_.tellg();
            is_succeed = (pos == std::ios::pos_type(0));
        }
    }

    if (is_succeed) {
        if (can_write) {
            stream_.seekp(0);
            std::ios::pos_type pos = stream_.tellp();
            is_succeed = (pos == std::ios::pos_type(0));
        }
    }

    auto size = 0LL;

    if (is_succeed) {
        if (end_pos != std::ios::pos_type(-1)) {
            can_seek = true;
            size = static_cast<int64_t>(end_pos - std::ios::pos_type(0));
        }
    }

    stream_.clear();
    size_ = size;
    can_read_ = can_read;
    can_seek_ = can_seek;
    can_write_ = can_write;

    return true;
}

// (virtual)
void FileStream::close()
{
    stream_.close();
    stream_.clear();

    size_ = 0;
    position_ = 0;
    can_read_ = false;
    can_seek_ = false;
    can_write_ = false;
    need_sync_read_position_ = false;
    need_sync_write_position_ = false;
}

// (virtual)
bool FileStream::is_open() const
{
    return stream_.is_open();
}

// (virtual)
int64_t FileStream::get_size()
{
    if (!is_open()) {
        return -1;
    }

    return size_;
}

// (virtual)
bool FileStream::set_size(
    int64_t size)
{
    if (!is_open()) {
        return false;
    }

    if (size < 0) {
        return false;
    }

    return false;
}

// (virtual)
bool FileStream::flush()
{
    if (!is_open()) {
        return false;
    }

    stream_.flush();
    auto result = !stream_.bad();
    stream_.clear();

    return result;
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

    auto new_position = 0LL;

    switch (origin) {
    case STREAM_SEEK_BEGIN:
        new_position = offset;
        break;

    case STREAM_SEEK_CURRENT:
        new_position = position_ + offset;
        break;

    case STREAM_SEEK_END:
        new_position = size_ + offset;
        break;

    default:
        return -1;
    }

    if (new_position < 0) {
        new_position = 0;
    }

    if (new_position != position_) {
        need_sync_read_position_ = true;
        need_sync_write_position_ = true;
    }

    position_ = new_position;

    return position_;
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

    if (can_seek() && need_sync_read_position_) {
        need_sync_read_position_ = false;

        stream_.seekg(position_);
        stream_.clear();
    }

    stream_.read(static_cast<char*>(buffer), count);
    stream_.clear();

    auto actual_count = static_cast<int>(stream_.gcount());

    if (can_seek() && actual_count > 0) {
        position_ += actual_count;
        need_sync_write_position_ = true;
    }

    return actual_count;
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

    if (can_seek() && need_sync_write_position_) {
        need_sync_write_position_ = false;

        stream_.seekp(position_);
        stream_.clear();
    }

    stream_.write(static_cast<const char*>(buffer), count);

    auto result = false;

    if (can_seek()) {
        if (stream_) {
            result = true;
            position_ += count;
        } else {
            result = false;
            auto cur_pos = stream_.tellp();
            position_ = static_cast<int64_t>(cur_pos - std::ios::pos_type(0));
        }

        if (position_ > size_) {
            size_ = position_;
        }

        need_sync_read_position_ = true;
    } else {
        result = !stream_;
    }

    stream_.clear();

    return result;
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
    std::ifstream stream(
        file_name.c_str(),
        std::ios_base::in | std::ios_base::binary | std::ios_base::ate);

    return stream.is_open();
}


} // namespace bstone
