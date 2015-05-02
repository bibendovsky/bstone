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
// A base class for streams.
//


#ifndef BSTONE_ISTREAM_INCLUDED
#define BSTONE_ISTREAM_INCLUDED


#include <cstdint>


namespace bstone {


enum class StreamSeekOrigin {
    begin,
    current,
    end,
}; // StreamSeekOrigin

enum class StreamOpenMode {
    read,
    write,
    read_write,
}; // StreamOpenMode


// A base class for streams.
class IStream {
public:
    IStream();

    virtual ~IStream();

    virtual void close();

    virtual bool is_open() const = 0;

    // Returns a size of the stream or a negative value on error.
    virtual int64_t get_size() = 0;

    // Sets a new size of the stream.
    // Returns false on error or if the stream is not seekable.
    virtual bool set_size(
        int64_t size) = 0;

    // Returns a new position or a negative value on error.
    virtual int64_t seek(
        int64_t offset,
        StreamSeekOrigin origin) = 0;

    // Skips a number of octets forward if count is positive or
    // backward otherwise.
    // Returns a negative value on error.
    virtual int64_t skip(
        int count);

    // Returns a current position or a negative value on error.
    virtual int64_t get_position();

    virtual bool set_position(
        int64_t position);

    // Reads a specified number of octets and returns an actual
    // read number of octets.
    virtual int read(
        void* buffer,
        int count) = 0;

    virtual bool write(
        const void* buffer,
        int count) = 0;

    // Reads a one octet and returns it.
    // Returns a negative value on error.
    virtual int read_octet();

    virtual bool write_octet(
        uint8_t value);

    // Writes a string without a terminator
    virtual bool write_string(
        const std::string& string);

    virtual bool can_read() const = 0;

    virtual bool can_seek() const = 0;

    virtual bool can_write() const = 0;

    // Copies the stream from a current position to
    // an another stream using internal buffer.
    bool copy_to(
        IStream* dst_stream,
        int buffer_size = 0);


    static int get_default_copy_buffer_size();
}; // IStream


} // bstone


#endif // BSTONE_ISTREAM_INCLUDED
