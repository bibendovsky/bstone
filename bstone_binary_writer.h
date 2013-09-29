#ifndef BSTONE_BINARY_WRITER_H
#define BSTONE_BINARY_WRITER_H


#include "bstone_istream.h"


namespace bstone {


class BinaryWriter {
public:
    BinaryWriter(
        IStream* stream = NULL);

    bool open(
        IStream* stream);

    void close();

    bool is_open() const;

    // Writes a signed 8-bit integer value.
    bool write_s8(
        Sint8 value);

    // Writes an unsigned 8-bit integer value.
    bool write_u8(
        Uint8 value);

    // Writes a signed 16-bit integer value.
    bool write_s16(
        Sint16 value);

    // Writes an unsigned 16-bit integer value.
    bool write_u16(
        Uint16 value);

    // Writes a signed 32-bit integer value.
    bool write_s32(
        Sint32 value);

    // Writes an unsigned 32-bit integer value.
    bool write_u32(
        Uint32 value);

    // Writes a signed 64-bit integer value.
    bool write_s64(
        Sint64 value);

    // Writes an unsigned 64-bit integer value.
    bool write_u64(
        Uint64 value);

    // Writes a 32-bit float-point value.
    bool write_r32(
        float value);

    // Writes a 64-bit float-point value.
    bool write_r64(
        double value);

    bool write(
        const void* buffer,
        int count);

    // Skips a number of octets forward if count is positive or
    // backward otherwise.
    // Returns false on error.
    bool skip(
        int count);

    // Returns a current position.
    Sint64 get_position() const;

    // Sets a current position to a specified one.
    bool set_position(
        Sint64 position);

    template<class T>
    bool write(
        const T& value)
    {
        if (!is_open())
            return false;

        return stream_->write(&value, sizeof(T));
    }

private:
    IStream* stream_;
}; // class BinaryWriter


} // namespace bstone


#endif // BSTONE_BINARY_WRITER_H
