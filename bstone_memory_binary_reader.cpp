//
// A binary reader for a block of memory.
//


#include "bstone_memory_binary_reader.h"


namespace bstone {


MemoryBinaryReader::MemoryBinaryReader() :
    data_(NULL),
    data_size_(0),
    data_offset_(0)
{
}

MemoryBinaryReader::MemoryBinaryReader(
    const void* data,
    Sint64 data_size)
{
    initialize(data, data_size);
}

bool MemoryBinaryReader::is_initialized() const
{
    return data_ != NULL;
}

bool MemoryBinaryReader::initialize(
    const void* data,
    Sint64 data_size)
{
    uninitialize();

    if (data == NULL)
        return false;

    if (data_size <= 0)
        return false;

    data_ = static_cast<const Uint8*>(data);
    data_size_ = data_size;
    data_offset_ = 0;

    return true;
}

void MemoryBinaryReader::uninitialize()
{
    data_ = NULL;
    data_size_ = 0;
    data_offset_ = 0;
}

Sint8 MemoryBinaryReader::read_s8()
{
    return read<int8_t>();
}

Uint8 MemoryBinaryReader::read_u8()
{
    return read<Uint8>();
}

Sint16 MemoryBinaryReader::read_s16()
{
    return read<Sint16>();
}

Uint16 MemoryBinaryReader::read_u16()
{
    return read<Uint16>();
}

Sint32 MemoryBinaryReader::read_s32()
{
    return read<Sint32>();
}

Uint32 MemoryBinaryReader::read_u32()
{
    return read<Uint32>();
}

Sint64 MemoryBinaryReader::read_s64()
{
    return read<Sint64>();
}

Uint64 MemoryBinaryReader::read_u64()
{
    return read<Uint64>();
}

float MemoryBinaryReader::read_r32()
{
    return read<float>();
}

double MemoryBinaryReader::read_r64()
{
    return read<double>();
}

bool MemoryBinaryReader::skip(
    Sint64 count)
{
    if (!is_initialized())
        return false;

    Sint64 new_offset = data_offset_ + count;

    if (new_offset < 0)
        return false;

    data_offset_ = new_offset;

    return true;
}

Sint64 MemoryBinaryReader::get_position() const
{
    if (!is_initialized())
        return false;

    return data_offset_;
}

bool MemoryBinaryReader::set_position(
    Sint64 position)
{
    if (!is_initialized())
        return false;

    if (position < 0)
        return false;

    data_offset_ = position;

    return true;
}


} // namespace bstone
