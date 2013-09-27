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

    stream_ = stream;

    return is_open();
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

bool BinaryReader::write(
    const void* buffer,
    int count)
{
    if (!is_open())
        return false;

    return stream_->write(buffer, count);
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
