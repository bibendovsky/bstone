//
// A binary reader for a block of memory.
//


#ifndef BSTONE_MEMORY_BINARY_READER_H
#define BSTONE_MEMORY_BINARY_READER_H


#include <cstddef>
#include <cstdint>


namespace bstone {


// A binary reader for a block of memory.
class MemoryBinaryReader {
public:
    MemoryBinaryReader();

    MemoryBinaryReader(
        const void* data,
        int64_t data_size);

    MemoryBinaryReader(
        const MemoryBinaryReader& that);

    ~MemoryBinaryReader();

    MemoryBinaryReader& operator=(
        const MemoryBinaryReader& that);

    // Initializes the reader.
    bool initialize(
        const void* data,
        int64_t data_size);

    // Uninitializes the reader.
    void uninitialize();

    // Returns true if the reader is initialized or
    // false otherwise.
    bool is_initialized() const;

    // Reads a signed 8-bit integer value.
    int8_t read_s8();

    // Reads an unsigned 8-bit integer value.
    uint8_t read_u8();

    // Reads a signed 16-bit integer value.
    int16_t read_s16();

    // Reads an unsigned 16-bit integer value.
    uint16_t read_u16();

    // Reads a signed 32-bit integer value.
    int32_t read_s32();

    // Reads an unsigned 32-bit integer value.
    uint32_t read_u32();

    // Reads a signed 64-bit integer value.
    int64_t read_s64();

    // Reads an unsigned 64-bit integer value.
    uint64_t read_u64();

    // Reads a 32-bit float-point value.
    float read_r32();

    // Reads a 64-bit float-point value.
    double read_r64();

    // Skips a specified number of octets.
    // Maybe be negative also.
    // Returns false on error.
    bool skip(
        int64_t count);

    // Returns a current position.
    int64_t get_position() const;

    // Sets a current position to a specified one.
    bool set_position(
        int64_t position);


private:
    const uint8_t* data_;
    int64_t data_size_;
    int64_t data_offset_;

    template<class T>
    T read()
    {
        if (!is_initialized())
            return 0;

        if (data_offset_ < 0)
            return 0;

        if ((data_offset_ + sizeof(T)) >= data_size_)
            return 0;

        T result = *reinterpret_cast<const T*>(&data_[data_offset_]);
        data_offset_ += sizeof(T);
        return result;
    }
}; // class MemoryBinaryReader


} // namespace bstone


#endif // BSTONE_MEMORY_BINARY_READER_H
