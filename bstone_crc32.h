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


#ifndef BSTONE_CRC32_INCLUDED
#define BSTONE_CRC32_INCLUDED


#include <cstdint>


namespace bstone {


class Crc32 {
public:
    Crc32();

    Crc32(
        const Crc32& that);

    Crc32& operator=(
        const Crc32& that);

    ~Crc32();


    void reset();

    uint32_t get() const;

    void update(
        const void* data,
        int size);

    template<typename T>
    void update(
        const T& value)
    {
        update(&value, static_cast<int>(sizeof(T)));
    }


private:
    uint32_t value_;
}; // Crc32


} // bstone


#endif // BSTONE_CRC32_INCLUDED
