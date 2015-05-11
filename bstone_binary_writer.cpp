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


#include "bstone_binary_writer.h"


namespace bstone {


BinaryWriter::BinaryWriter(
    IStream* stream) :
        stream_()
{
    open(stream);
}

bool BinaryWriter::open(
    IStream* stream)
{
    close();

    if (!stream) {
        return false;
    }

    if (!stream->can_write()) {
        return false;
    }

    stream_ = stream;

    return true;
}

void BinaryWriter::close()
{
    stream_ = nullptr;
}

bool BinaryWriter::is_open() const
{
    return stream_ != nullptr;
}

bool BinaryWriter::write_s8(
    int8_t value)
{
    return write(value);
}

bool BinaryWriter::write_u8(
    uint8_t value)
{
    return write(value);
}

bool BinaryWriter::write_s16(
    int16_t value)
{
    return write(value);
}

bool BinaryWriter::write_u16(
    uint16_t value)
{
    return write(value);
}

bool BinaryWriter::write_s32(
    int32_t value)
{
    return write(value);
}

bool BinaryWriter::write_u32(
    uint32_t value)
{
    return write(value);
}

bool BinaryWriter::write_s64(
    int64_t value)
{
    return write(value);
}

bool BinaryWriter::write_u64(
    uint64_t value)
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
    if (!is_open()) {
        return false;
    }

    return stream_->write(buffer, count);
}

bool BinaryWriter::skip(
    int count)
{
    if (!is_open()) {
        return false;
    }

    return stream_->skip(count) >= 0;
}

int64_t BinaryWriter::get_position() const
{
    if (!is_open()) {
        return 0;
    }

    return stream_->get_position();
}

bool BinaryWriter::set_position(
    int64_t position)
{
    if (!is_open()) {
        return false;
    }

    return stream_->set_position(position);
}


} // bstone
