#include "bstone_istream.h"

#include <vector>

#include "bstone_un_value.h"


namespace bstone {


IStream::IStream()
{
}

// (virtual)
IStream::~IStream()
{
    close();
}

// (virtual)
void IStream::close()
{
}

// (virtual)
Sint64 IStream::skip(
    int count)
{
    return seek(count, STREAM_SEEK_CURRENT);
}

// (virtual)
Sint64 IStream::get_position()
{
    return seek(0, STREAM_SEEK_CURRENT);
}

// (virtual)
bool IStream::set_position(
    Sint64 position)
{
    return seek(position) >= 0;
}

// (virtual)
int IStream::read_octet()
{
    Uint8 value = 0;

    if (read(&value, 1) > 0)
        return value;
    else
        return 0;
}

// (virtual)
bool IStream::write_octet(
    Uint8 value)
{
    return write(&value, 1);
}

bool IStream::copy_to(
    IStream* dst_stream,
    int buffer_size)
{
    if (dst_stream == NULL)
        return false;

    if (!dst_stream->can_write())
        return false;

    if (buffer_size <= 0)
        buffer_size = get_default_copy_buffer_size();

    std::vector<UnValue<char>> buffer(buffer_size);

    for (int count = -1; count != 0; ) {
        count = read(&buffer[0], buffer_size);

        if (!dst_stream->write(&buffer[0], count))
            return false;
    }

    return true;
}

// (static)
int IStream::get_default_copy_buffer_size()
{
    return 4096;
}


} // namespace bstone
