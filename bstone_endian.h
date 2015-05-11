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
// Byte order (endianness) manipulation.
//


#ifndef BSTONE_ENDIAN_INCLUDED
#define BSTONE_ENDIAN_INCLUDED


#include <algorithm>
#include "SDL.h"


namespace bstone {


// Byte order (endianness) manipulation.
class Endian {
public:
    Endian() = delete;

    Endian(
        const Endian& that) = delete;

    Endian& operator=(
        const Endian& that) = delete;

    ~Endian() = delete;


    // Swaps bytes on non little-endian system.
    template<typename T>
    static T le(
        const T& value)
    {
        return le(value, DefaultOrderTag());
    }

    // Copies an array of elements with swapped bytes into another array
    // on non little-endian system.
    template<typename T, size_t N>
    static void le(
        const T (&src_data)[N],
        T (&dst_data)[N])
    {
        for (size_t i = 0; i < N; ++i) {
            dst_data[i] = le(src_data[i]);
        }
    }

    // Copies an array of elements with swapped bytes into another array
    // on non little-endian system.
    template<class T>
    static void le(
        const T* src_data,
        size_t count,
        T* dst_data)
    {
        for (size_t i = 0; i < count; ++i) {
            dst_data[i] = le(src_data[i]);
        }
    }

    // Swaps bytes on non big-endian system.
    template<typename T>
    static T be(
        const T& value)
    {
        return be(value, DefaultOrderTag());
    }

    // Copies an array of elements with swapped bytes into another array
    // on non big-endian system.
    template<typename T, size_t N>
    static void be(
        const T (&src_data)[N],
        T (&dst_data)[N])
    {
        for (size_t i = 0; i < N; ++i) {
            dst_data[i] = be(src_data[i]);
        }
    }

    // Copies an array of elements with swapped bytes into another array
    // on non big-endian system.
    template<typename T>
    static void be(
        const T* src_data,
        size_t count,
        T* dst_data)
    {
        for (size_t i = 0; i < count; ++i) {
            dst_data[i] = be(src_data[i]);
        }
    }

    // Swaps bytes in place on non little-endian system.
    template<typename T>
    static void lei(
        T& value)
    {
        lei(value, DefaultOrderTag());
    }

    // Swaps bytes in place of an array of elements
    // on non little-endian system.
    template<typename T, size_t N>
    static void lei(
        T (&data)[N])
    {
        for (size_t i = 0; i < N; ++i) {
            lei(data[i]);
        }
    }

    // Swaps bytes in place of an array of elements
    // on non little-endian system.
    template<typename T>
    static void lei(
        T* data,
        size_t count)
    {
        for (size_t i = 0; i < count; ++i) {
            lei(data[i]);
        }
    }

    // Swaps bytes in place on non big-endian system.
    template<typename T>
    static void bei(
        T& value)
    {
        bei(value, DefaultOrderTag());
    }

    // Swaps bytes in place of an array of elements
    // on non big-endian system.
    template<typename T, size_t N>
    static void bei(
        T (&data)[N])
    {
        for (size_t i = 0; i < N; ++i) {
            bei(data[i]);
        }
    }

    // Swaps bytes in place of an array of elements
    // on non big-endian system.
    template<typename T>
    static void bei(
        T* data,
        size_t count)
    {
        for (size_t i = 0; i < count; ++i) {
            bei(data[i]);
        }
    }


private:
    template<int TOrder>
    class OrderTag {
    public:
    }; // OrderTag


    using LeOrderTag = OrderTag<SDL_LIL_ENDIAN>;
    using BeOrderTag = OrderTag<SDL_BIG_ENDIAN>;
    using DefaultOrderTag = OrderTag<SDL_BYTEORDER>;


    template<typename T>
    static T le(
        const T& value,
        LeOrderTag)
    {
        return value;
    }

    template<typename T>
    static T le(
        const T& value,
        BeOrderTag)
    {
        return le_be(value);
    }


    template<typename T>
    static T be(
        const T& value,
        LeOrderTag)
    {
        return le_be(value);
    }

    template<typename T>
    static T be(
        const T& value,
        BeOrderTag)
    {
        return value;
    }


    template<typename T>
    static void lei(
        T& value,
        LeOrderTag)
    {
        static_cast<void>(value);
    }

    template<typename T>
    static void lei(
        T& value,
        BeOrderTag)
    {
        lei_bei(value);
    }


    template<typename T>
    static void bei(
        T& value,
        LeOrderTag)
    {
        lei_bei(value);
    }

    template<typename T>
    static void bei(
        T& value,
        BeOrderTag)
    {
        static_cast<void>(value);
    }


    // Swaps bytes.
    template<class T>
    static T le_be(
        const T& value)
    {
        auto result = value;
        lei_bei(result);
        return result;
    }

    // Swaps bytes in place.
    template<class T>
    static void lei_bei(
        T& value)
    {
        for (
            size_t i = 0, j = sizeof(T) - 1, n = sizeof(T) / 2;
            i < n;
            ++i, --j)
        {
            std::swap(
                reinterpret_cast<char*>(&value)[i],
                reinterpret_cast<char*>(&value)[j]);
        }
    }
}; // Endian


} // bstone


#endif // BSTONE_ENDIAN_INCLUDED
