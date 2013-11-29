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


#include "bstone_binary_writer.h"


namespace bstone {


BinaryWriter::BinaryWriter(
    IStream* stream) :
        stream_(NULL)
{
    open(stream);
}

bool BinaryWriter::open(
    IStream* stream)
{
    close();

    if (stream == NULL)
        return false;

    if (!stream->can_write())
        return false;

    stream_ = stream;

    return true;
}

void BinaryWriter::close()
{
    stream_ = NULL;
}

bool BinaryWriter::is_open() const
{
    return stream_ != NULL;
}

bool BinaryWriter::write_s8(
    Sint8 value)
{
    return write(value);
}

bool BinaryWriter::write_u8(
    Uint8 value)
{
    return write(value);
}

bool BinaryWriter::write_s16(
    Sint16 value)
{
    return write(value);
}

bool BinaryWriter::write_u16(
    Uint16 value)
{
    return write(value);
}

bool BinaryWriter::write_s32(
    Sint32 value)
{
    return write(value);
}

bool BinaryWriter::write_u32(
    Uint32 value)
{
    return write(value);
}

bool BinaryWriter::write_s64(
    Sint64 value)
{
    return write(value);
}

bool BinaryWriter::write_u64(
    Uint64 value)
{
    return write(value);
}

bool BinaryWriter::write_r32(
    float value)
{
    return write(value);
}

bool BinaryWriter::write_r64(
    double value)
{
    return write(value);
}

bool BinaryWriter::write(
    const void* buffer,
    int count)
{
    if (!is_open())
        return false;

    return stream_->write(buffer, count);
}

bool BinaryWriter::skip(
    int count)
{
    if (!is_open())
        return false;

    return stream_->skip(count) >= 0;
}

Sint64 BinaryWriter::get_position() const
{
    if (!is_open())
        return false;

    return stream_->get_position();
}

bool BinaryWriter::set_position(
    Sint64 position)
{
    if (!is_open())
        return false;

    return stream_->set_position(position);
}


} // namespace bstone
