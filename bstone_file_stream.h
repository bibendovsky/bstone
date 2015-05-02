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
// File stream.
//


#ifndef BSTONE_FILE_STREAM_INCLUDED
#define BSTONE_FILE_STREAM_INCLUDED


#include "bstone_istream.h"


namespace bstone {


// File stream.
class FileStream :
    public IStream
{
public:
    FileStream();

    FileStream(
        const std::string& file_name,
        StreamOpenMode open_mode = StreamOpenMode::read);

    FileStream(
        const FileStream& that) = delete;

    FileStream& operator=(
        const FileStream& that) = delete;

    virtual ~FileStream();

    bool open(
        const std::string& file_name,
        StreamOpenMode open_mode = StreamOpenMode::read);

    virtual void close();

    virtual bool is_open() const;

    virtual int64_t get_size();

    virtual bool set_size(
        int64_t size);

    virtual int64_t seek(
        int64_t offset,
        StreamSeekOrigin origin);

    virtual int read(
        void* buffer,
        int count);

    virtual bool write(
        const void* buffer,
        int count);

    virtual bool can_read() const;

    virtual bool can_seek() const;

    virtual bool can_write() const;

    static bool is_exists(
        const std::string& file_name);


private:
    void* context_;
    bool can_read_;
    bool can_seek_;
    bool can_write_;
}; // FileStream


} // bstone


#endif // BSTONE_FILE_STREAM_INCLUDED
