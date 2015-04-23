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


#include "bstone_string_helper.h"
#include <algorithm>
#include <locale>


namespace {


class Internals {
public:
    using CType = std::ctype<char>;


    Internals() = delete;

    Internals(
        const Internals& that) = delete;

    Internals& operator=(
        const Internals& that) = delete;

    ~Internals() = delete;


    static const CType& get_ctype_facet()
    {
        static std::locale locale;
        static const auto& result = std::use_facet<CType>(locale);
        return result;
    }

    static char to_lower(
        char value)
    {
        return get_ctype_facet().tolower(value);
    }
}; // Internals


} // namespace


namespace bstone {


// (static)
char StringHelper::to_lower(
    char value)
{
    return Internals::to_lower(value);
}

// (static)
std::string StringHelper::to_lower(
    const std::string& value)
{
    auto result = value;

    std::transform(
        result.begin(),
        result.end(),
        result.begin(),
        Internals::to_lower);

    return result;
}

// (static)
bool StringHelper::is_iequal(
    const std::string& a,
    const std::string& b)
{
    auto result = std::mismatch(
        a.cbegin(),
        a.cend(),
        b.cbegin(),
        [] (char a, char b)
        {
            return
                Internals::get_ctype_facet().tolower(a) ==
                Internals::get_ctype_facet().tolower(b);
        }
    );

    return result.first == a.cend();
}

// (static)
bool StringHelper::is(
    std::ctype_base::mask mask,
    char value)
{
    return Internals::get_ctype_facet().is(mask, value);
}

// (static)
const std::string& StringHelper::get_empty()
{
    static const std::string result;
    return result;
}


} // bstone
