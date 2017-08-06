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
// A binary reader for a block of memory.
//


#include "bstone_memory_binary_reader.h"
#include "bstone_endian.h"
#include <memory>

namespace bstone {


MemoryBinaryReader::MemoryBinaryReader() :
        data_(),
        data_size_(),
        data_offset_()
{
}

MemoryBinaryReader::MemoryBinaryReader(
    const void* data,
    int64_t data_size)
{
    open(data, data_size);
}

bool MemoryBinaryReader::is_initialized() const
{
    return data_ != nullptr;
}

bool MemoryBinaryReader::open(
    const void* data,
    int64_t data_size)
{
    close();

    if (!data) {
        return false;
    }

    if (data_size <= 0) {
        return false;
    }

    data_ = static_cast<const uint8_t*>(data);
    data_size_ = data_size;
    data_offset_ = 0;

    return true;
}

void MemoryBinaryReader::close()
{
    data_ = nullptr;
    data_size_ = 0;
    data_offset_ = 0;
}

int8_t MemoryBinaryReader::read_s8()
{
    return read<int8_t>();
}

uint8_t MemoryBinaryReader::read_u8()
{
    return read<uint8_t>();
}

int16_t MemoryBinaryReader::read_s16()
{
    return read<int16_t>();
}

uint16_t MemoryBinaryReader::read_u16()
{
    return read<uint16_t>();
}

int32_t MemoryBinaryReader::read_s32()
{
    return read<int32_t>();
}

uint32_t MemoryBinaryReader::read_u32()
{
    return read<uint32_t>();
}

int64_t MemoryBinaryReader::read_s64()
{
    return read<int64_t>();
}

uint64_t MemoryBinaryReader::read_u64()
{
    return read<uint64_t>();
}

float MemoryBinaryReader::read_r32()
{
    return read<float>();
}

double MemoryBinaryReader::read_r64()
{
    return read<double>();
}

std::string MemoryBinaryReader::read_string()
{
    auto length = bstone::Endian::le(read_s32());

    std::string string(length, '\0');

    if (length > 0) {
        if (!read(&string[0], length)) {
            string.clear();
        }
    }

    return string;
}

bool MemoryBinaryReader::read(
    void* buffer,
    int count)
{
    if (!buffer) {
        return false;
    }

    if (count <= 0) {
        return true;
    }

    if (!is_initialized()) {
        return false;
    }

    if (data_offset_ < 0) {
        return false;
    }

    if ((data_offset_ + count) >= data_size_) {
        return false;
    }

    std::uninitialized_copy_n(
        &data_[data_offset_],
        count,
        static_cast<uint8_t*>(buffer));

    data_offset_ += count;

    return true;
}

bool MemoryBinaryReader::skip(
    int64_t count)
{
    if (!is_initialized()) {
        return false;
    }

    auto new_offset = data_offset_ + count;

    if (new_offset < 0) {
        return false;
    }

    data_offset_ = new_offset;

    return true;
}

int64_t MemoryBinaryReader::get_position() const
{
    return data_offset_;
}

bool MemoryBinaryReader::set_position(
    int64_t position)
{
    if (!is_initialized()) {
        return false;
    }

    if (position < 0) {
        return false;
    }

    data_offset_ = position;

    return true;
}


} // bstone
