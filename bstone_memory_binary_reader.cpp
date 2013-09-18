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
    int64_t data_size)
{
    initialize(data, data_size);
}

bool MemoryBinaryReader::is_initialized() const
{
    return data_ != NULL;
}

bool MemoryBinaryReader::initialize(
    const void* data,
    int64_t data_size)
{
    uninitialize();

    if (data == NULL)
        return false;

    if (data_size <= 0)
        return false;

    data_ = static_cast<const uint8_t*>(data);
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

bool MemoryBinaryReader::skip(int64_t count)
{
    if (!is_initialized())
        return false;

    int64_t new_offset = data_offset_ + count;

    if (new_offset < 0)
        return false;

    data_offset_ = new_offset;

    return true;
}

int64_t MemoryBinaryReader::get_position() const
{
    if (!is_initialized())
        return false;

    return data_offset_;
}

bool MemoryBinaryReader::set_position(
    int64_t position)
{
    if (!is_initialized())
        return false;

    if (position < 0)
        return false;

    data_offset_ = position;

    return true;
}


} // namespace bstone
