/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


//
// A text writer for a stream.
//


#include "bstone_memory_text_writer.h"


namespace bstone
{


MemoryTextWriter::MemoryTextWriter()
    :
    stream_{}
{
}

MemoryTextWriter::MemoryTextWriter(
    IStream* stream)
    :
    MemoryTextWriter{}
{
    static_cast<void>(open(stream));
}


bool MemoryTextWriter::open(
    IStream* stream)
{
    if (!stream)
    {
        return false;
    }

    stream_ = stream;

    return true;
}

void MemoryTextWriter::close()
{
    stream_ = nullptr;
}

bool MemoryTextWriter::is_initialized() const
{
    return stream_ != nullptr;
}

bool MemoryTextWriter::write(
    const std::string& string)
{
    if (!is_initialized())
    {
        return false;
    }

    if (string.empty())
    {
        return true;
    }

    return stream_->write(string.data(), static_cast<int>(string.length()));
}

bool MemoryTextWriter::write_line(
    const std::string& string)
{
    static const std::string new_line = "\n";

    if (!write(string))
    {
        return false;
    }

    if (!write(new_line))
    {
        return false;
    }

    return true;
}


} // bstone
