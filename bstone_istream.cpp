#include "bstone_istream.h"

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


} // namespace bstone
