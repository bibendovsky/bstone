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
// A wrapper for C platform-dependent functions.
//


#ifndef BSTONE_C_H
#define BSTONE_C_H


#include <sstream>


namespace bstone {


// A wrapper for C platform-dependent functions.
class C {
public:
    template<class T>
    static char* xitoa(
        T value,
        char* str,
        int radix)
    {
        if (!str) {
            return nullptr;
        }

        str[0] = '\0';

        std::ios::fmtflags flags;

        switch (radix) {
        case 8:
            flags = std::ios::oct;
            break;

        case 10:
            flags = std::ios::dec;
            break;

        case 16:
            flags = std::ios::hex;
            break;

        default:
            return str;
        }

        std::ostringstream iss;
        iss.setf(flags, std::ios::oct | std::ios::dec | std::ios::hex);
        iss << value;

        if (iss) {
            std::string value_str = iss.str();
            std::string::traits_type::copy(
                str,
                value_str.c_str(),
                value_str.size());
            str[value_str.size()] = '\0';
        }

        return str;
    }

    static int memicmp(
        const void* buf1,
        const void* buf2,
        size_t cnt);

    static int stricmp(
        const char* str1,
        const char* str2);

    static double m_pi();
}; // class C


} // namespace bstone


#endif // BSTONE_C_H

