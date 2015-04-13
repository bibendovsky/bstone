/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

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
============================================================== */


//
// A memory stream.
//


#ifndef BSTONE_MEMORY_STREAM_INCLUDED
#define BSTONE_MEMORY_STREAM_INCLUDED


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
        const uint8_t* buffer,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    MemoryStream(
        const MemoryStream& that) = delete;

    MemoryStream& operator=(
        const MemoryStream& that) = delete;

    virtual ~MemoryStream();


    bool open(
        int initial_capacity = 0,
        StreamOpenMode open_mode = STREAM_OPEN_READ_WRITE);

    bool open(
        int buffer_size,
        int buffer_offset,
        const uint8_t* buffer,
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    virtual void close();

    virtual bool is_open() const;

    virtual int64_t get_size();

    virtual bool set_size(
        int64_t size);

    virtual bool flush();

    virtual int64_t seek(
        int64_t offset,
        StreamSeekOrigin origin = STREAM_SEEK_BEGIN);

    virtual int64_t get_position();

    virtual int read(
        void* buffer,
        int count);

    virtual bool write(
        const void* buffer,
        int count);

    virtual bool can_read() const;

    virtual bool can_seek() const;

    virtual bool can_write() const;

    uint8_t* get_data();

    const uint8_t* get_data() const;

    bool remove_block(
        int64_t offset,
        int count);

private:
    using Buffer = std::vector<UnValue<uint8_t>>;

    bool is_open_;
    bool can_read_;
    bool can_write_;
    int64_t position_;
    int64_t size_;
    int64_t ext_size_;
    uint8_t* buffer_;
    uint8_t* ext_buffer_;
    Buffer int_buffer_;
}; // IStream


} // bstone


#endif // BSTONE_MEMORY_STREAM_INCLUDED
