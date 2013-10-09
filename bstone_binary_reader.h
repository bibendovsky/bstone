#ifndef BSTONE_BINARY_READER_H
#define BSTONE_BINARY_READER_H


#include "bstone_istream.h"


namespace bstone {


class BinaryReader {
public:
    BinaryReader(
        IStream* stream = NULL);

    bool open(
        IStream* stream);

    void close();

    bool is_open() const;

    // Reads a signed 8-bit integer value.
    Sint8 read_s8();

    // Reads an unsigned 8-bit integer value.
    Uint8 read_u8();

    // Reads a signed 16-bit integer value.
    Sint16 read_s16();

    // Reads an unsigned 16-bit integer value.
    Uint16 read_u16();

    // Reads a signed 32-bit integer value.
    Sint32 read_s32();

    // Reads an unsigned 32-bit integer value.
    Uint32 read_u32();

    // Reads a signed 64-bit integer value.
    Sint64 read_s64();

    // Reads an unsigned 64-bit integer value.
    Uint64 read_u64();

    // Reads a 32-bit float-point value.
    float read_r32();

    // Reads a 64-bit float-point value.
    double read_r64();

    bool read(
        void* buffer,
        int count);

    template<class T>
    bool read(
        T& value)
    {
        if (!is_open())
            return false;

        return stream_->read(&value, sizeof(T)) == sizeof(T);
    }

    template<class T,size_t N>
    bool read(
        T (&value)[N])
    {
        if (!is_open())
            return false;

        return stream_->read(value, N * sizeof(T)) == N * sizeof(T);
    }

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

private:
    IStream* stream_;

    template<class T>
    T read()
    {
        if (!is_open())
            return 0;

        T value;

        if (stream_->read(&value, sizeof(T)) != sizeof(T))
            return 0;

        return value;
    }
}; // class BinaryReader


} // namespace bstone


#endif // BSTONE_BINARY_READER_H
