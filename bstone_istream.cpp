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
}

// (virtual)
void IStream::close()
{
}

// (virtual)
int64_t IStream::skip(
    int count)
{
    return seek(count, StreamSeekOrigin::current);
}

// (virtual)
int64_t IStream::get_position()
{
    return seek(0, StreamSeekOrigin::current);
}

// (virtual)
bool IStream::set_position(
    int64_t position)
{
    return seek(position, StreamSeekOrigin::begin) >= 0;
}

// (virtual)
int IStream::read_octet()
{
    auto value = uint8_t(0);

    if (read(&value, 1) > 0) {
        return value;
    } else {
        return 0;
    }
}

// (virtual)
bool IStream::write_octet(
    uint8_t value)
{
    return write(&value, 1);
}

// (virtual)
bool IStream::write_string(
    const std::string& string)
{
    return write(
        string.c_str(),
        static_cast<int>(string.length()));
}

bool IStream::copy_to(
    IStream* dst_stream,
    int buffer_size)
{
    if (!dst_stream) {
        return false;
    }

    if (!dst_stream->can_write()) {
        return false;
    }

    if (buffer_size <= 0) {
        buffer_size = get_default_copy_buffer_size();
    }

    std::vector<UnValue<char>> buffer(buffer_size);

    for (auto count = -1; count != 0; ) {
        count = read(&buffer[0], buffer_size);

        if (!dst_stream->write(&buffer[0], count)) {
            return false;
        }
    }

    return true;
}

// (static)
int IStream::get_default_copy_buffer_size()
{
    return 4096;
}


} // namespace bstone
