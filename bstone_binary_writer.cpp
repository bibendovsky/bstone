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
