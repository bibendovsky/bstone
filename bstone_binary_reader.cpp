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


#include "bstone_binary_reader.h"


namespace bstone {


BinaryReader::BinaryReader(
    IStream* stream) :
        stream_(NULL)
{
    open(stream);
}

bool BinaryReader::open(
    IStream* stream)
{
    close();

    if (stream == NULL)
        return false;

    if (!stream->can_read())
        return false;

    stream_ = stream;

    return true;
}

void BinaryReader::close()
{
    stream_ = NULL;
}

bool BinaryReader::is_open() const
{
    return stream_ != NULL;
}

Sint8 BinaryReader::read_s8()
{
    return read<Sint8>();
}

Uint8 BinaryReader::read_u8()
{
    return read<Uint8>();
}

Sint16 BinaryReader::read_s16()
{
    return read<Sint16>();
}

Uint16 BinaryReader::read_u16()
{
    return read<Uint16>();
}

Sint32 BinaryReader::read_s32()
{
    return read<Sint32>();
}

Uint32 BinaryReader::read_u32()
{
    return read<Uint32>();
}

Sint64 BinaryReader::read_s64()
{
    return read<Sint64>();
}

Uint64 BinaryReader::read_u64()
{
    return read<Uint64>();
}

float BinaryReader::read_r32()
{
    return read<float>();
}

double BinaryReader::read_r64()
{
    return read<double>();
}

bool BinaryReader::read(
    void* buffer,
    int count)
{
    if (!is_open())
        return false;

    return stream_->read(buffer, count) == count;
}

bool BinaryReader::skip(
    int count)
{
    if (!is_open())
        return false;

    return stream_->skip(count) >= 0;
}

Sint64 BinaryReader::get_position() const
{
    if (!is_open())
        return false;

    return stream_->get_position();
}

bool BinaryReader::set_position(
    Sint64 position)
{
    if (!is_open())
        return false;

    return stream_->set_position(position);
}


} // namespace bstone
