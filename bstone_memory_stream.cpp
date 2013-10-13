#include "bstone_memory_stream.h"

#include <algorithm>

namespace bstone {


MemoryStream::MemoryStream(
    int initial_capacity,
    StreamOpenMode open_mode) :
        is_open_(false),
        can_read_(false),
        can_write_(false),
        position_(0),
        size_(0),
        ext_size_(0),
        buffer_(NULL),
        ext_buffer_(NULL)
{
    open(initial_capacity, open_mode);
}

MemoryStream::MemoryStream(
    int buffer_size,
    int buffer_offset,
    const Uint8* buffer,
    StreamOpenMode open_mode) :
        is_open_(false),
        can_read_(false),
        can_write_(false),
        position_(0),
        size_(0),
        ext_size_(0),
        buffer_(NULL),
        ext_buffer_(NULL)
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

    if ((open_mode & STREAM_OPEN_READ_WRITE) == 0)
        return false;

    if ((open_mode & STREAM_OPEN_TRUNCATE) != 0)
        return false;

    if (initial_capacity < 0)
        initial_capacity = 0;

    int_buffer_.reserve(initial_capacity);

    is_open_ = true;
    can_read_ = ((open_mode & STREAM_OPEN_READ) != 0);
    can_write_ = ((open_mode & STREAM_OPEN_WRITE) != 0);

    return true;
}

bool MemoryStream::open(
    int buffer_size,
    int buffer_offset,
    const Uint8* buffer,
    StreamOpenMode open_mode)
{
    close();

    if (buffer_size < 0)
        buffer_size = 0;

    if (buffer == NULL)
        return false;

    if ((open_mode & STREAM_OPEN_READ_WRITE) == 0)
        return false;

    if ((open_mode & STREAM_OPEN_TRUNCATE) != 0)
        return false;

    is_open_ = true;
    can_read_ = ((open_mode & STREAM_OPEN_READ) != 0);
    can_write_ = ((open_mode & STREAM_OPEN_WRITE) != 0);
    size_ = buffer_size;
    ext_size_ = buffer_size;
    buffer_ = const_cast<Uint8*>(&buffer[buffer_offset]);
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
    buffer_ = NULL;
    ext_buffer_ = NULL;
    Buffer().swap(int_buffer_);
}

// (virtual)
bool MemoryStream::is_open() const
{
    return is_open_;
}

// (virtual)
Sint64 MemoryStream::get_size()
{
    if (!is_open())
        return 0;

    return size_;
}

// (virtual)
bool MemoryStream::set_size(
    Sint64 size)
{
    if (!is_open())
        return false;

    if (!can_write())
        return false;

    if (size < 0)
        return false;

    if (ext_buffer_ != NULL)
        return false;

    int_buffer_.resize(static_cast<size_t>(size));

    size_ = size;

    if (size_ > 0)
        buffer_ = reinterpret_cast<Uint8*>(&int_buffer_[0]);
    else
        buffer_ = NULL;

    return true;
}

// (virtual)
bool MemoryStream::flush()
{
    return is_open();
}

// (virtual)
Sint64 MemoryStream::seek(
    Sint64 offset,
    StreamSeekOrigin origin)
{
    if (!is_open())
        return -1;

    if (!can_seek())
        return -1;

    switch (origin) {
    case STREAM_SEEK_BEGIN:
        position_ = offset;
        break;

    case STREAM_SEEK_CURRENT:
        position_ += offset;
        break;

    case STREAM_SEEK_END:
        position_ = size_ + offset;
        break;

    default:
        return -1;
    }

    if (position_ < 0)
        position_ = 0;

    return position_;
}

// (virtual)
Sint64 MemoryStream::get_position()
{
    if (!is_open())
        return -1;

    if (!can_seek())
        return -1;

    return position_;
}

// (virtual)
int MemoryStream::read(
    void* buffer,
    int count)
{
    if (!is_open())
        return 0;

    if (!can_read())
        return 0;

    if (buffer == NULL)
        return 0;

    if (count <= 0)
        return 0;

    Sint64 remain = size_ - position_;

    if (remain <= 0)
        return 0;

    int read_count = static_cast<int>(std::min(
        static_cast<Sint64>(count), remain));

    std::uninitialized_copy(
        &buffer_[position_],
        &buffer_[position_ + read_count],
        static_cast<Uint8*>(buffer));

    position_ += read_count;

    return read_count;
}

// (virtual)
bool MemoryStream::write(
    const void* buffer,
    int count)
{
    if (!is_open())
        return false;

    if (!can_write())
        return false;

    if (count < 0)
        return false;

    if (count == 0)
        return true;

    if (buffer == NULL)
        return false;

    if (ext_buffer_ == NULL) {
        Sint64 new_size = position_ + count;

        if (new_size > size_) {
            int_buffer_.resize(static_cast<size_t>(new_size));

            size_ = new_size;
            buffer_ = reinterpret_cast<Uint8*>(&int_buffer_[0]);
        }
    } else {
        if ((position_ + count) > ext_size_)
            return false;
    }

    std::uninitialized_copy(
        static_cast<const Uint8*>(buffer),
        &static_cast<const Uint8*>(buffer)[count],
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

Uint8* MemoryStream::get_data()
{
    if (!is_open())
        return NULL;

    return buffer_;
}

const Uint8* MemoryStream::get_data() const
{
    if (!is_open())
        return NULL;

    return buffer_;
}

bool MemoryStream::remove_block(
    Sint64 offset,
    int count)
{
    if (!is_open())
        return false;

    if (offset < 0)
        return false;

    if (count < 0)
        return false;

    if (count == 0)
        return true;

    if ((offset + count) > size_)
        return false;

    int_buffer_.erase(
        int_buffer_.begin() + offset,
        int_buffer_.begin() + offset + count);

    size_ -= count;

    return true;
}


} // namespace bstone
