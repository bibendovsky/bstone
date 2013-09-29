//
// A memory stream.
//


#ifndef BSTONE_MEMORY_STREAM_H
#define BSTONE_MEMORY_STREAM_H


#include <vector>

#include "bstone_istream.h"
#include "bstone_un_value.h"


namespace bstone {


// A memory stream.
class MemoryStream :
    public IStream
{
public:
    MemoryStream(
        int initial_capacity = 0,
        StreamOpenMode open_mode = STREAM_OPEN_READ_WRITE);

    MemoryStream(
        int buffer_size,
        int buffer_offset,
        const Uint8* buffer,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    bool open(
        int initial_capacity = 0,
        StreamOpenMode open_mode = STREAM_OPEN_READ_WRITE);

    bool open(
        int buffer_size,
        int buffer_offset,
        const Uint8* buffer,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    virtual void close();

    virtual bool is_open() const;

    virtual Sint64 get_size();

    virtual bool set_size(
        Sint64 size);

    virtual bool flush();

    virtual Sint64 seek(
        Sint64 offset,
        StreamSeekOrigin origin = STREAM_SEEK_BEGIN);

    virtual Sint64 get_position();

    virtual int read(
        void* buffer,
        int count);

    virtual bool write(
        const void* buffer,
        int count);

    virtual bool can_read() const;

    virtual bool can_seek() const;

    virtual bool can_write() const;

    Uint8* get_data();

    const Uint8* get_data() const;

    bool remove_block(
        Sint64 offset,
        int count);

private:
    typedef std::vector<UnValue<Uint8>> Buffer;

    bool is_open_;
    bool can_read_;
    bool can_write_;
    Sint64 position_;
    Sint64 size_;
    Sint64 ext_size_;
    Uint8* buffer_;
    Uint8* ext_buffer_;
    Buffer int_buffer_;
}; // class IStream


} // namespace bstone


#endif // BSTONE_MEMORY_STREAM_H
