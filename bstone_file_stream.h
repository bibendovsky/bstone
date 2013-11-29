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


#ifndef BSTONE_FILE_STREAM_H
#define BSTONE_FILE_STREAM_H


#include <fstream>

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
        StreamOpenMode open_mode = STREAM_OPEN_READ);

    virtual ~FileStream();

    bool open(
        const std::string& file_name,
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

    virtual int read(
        void* buffer,
        int count);

    virtual bool write(
        const void* buffer,
        int count);

    virtual bool can_read() const;

    virtual bool can_seek() const;

    virtual bool can_write() const;

private:
    std::fstream stream_;
    Sint64 size_;
    Sint64 position_;
    bool can_read_;
    bool can_seek_;
    bool can_write_;
    bool need_sync_read_position_;
    bool need_sync_write_position_;

    FileStream(
        const FileStream& that);

    FileStream& operator=(
        const FileStream& that);
}; // class FileStream


} // namespace bstone


#endif // BSTONE_FILE_STREAM_H
