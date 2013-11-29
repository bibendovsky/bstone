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
    typedef std::ctype<char> CType;


    static const CType& get_ctype_facet()
    {
        static std::locale locale("");

        static const CType& result =
            std::use_facet<CType>(locale);

        return result;
    }

    static bool icompare_predicate(
        char a,
        char b)
    {
        return get_ctype_facet().tolower(a) ==
            get_ctype_facet().tolower(b);
    }

private:
    Internals();

    Internals(
        const Internals& that);

    ~Internals();

    Internals& operator=(
        const Internals& that);
}; // class Internals


} // namespace


namespace bstone {


// (static)
char StringHelper::to_lower(
    char value)
{
    return Internals::get_ctype_facet().tolower(value);
}

// (static)
std::string StringHelper::to_lower(
    const std::string& value)
{
    std::string result(value);

    if (!value.empty()) {
        Internals::get_ctype_facet().tolower(
            &result[0], &(&result[0])[value.size()]);
    }

    return result;
}

// (static)
bool StringHelper::is_iequal(
    const std::string& a,
    const std::string& b)
{
    std::pair<const char*,const char*> result = std::mismatch(
        a.c_str(),
        &(a.c_str())[a.size()],
        b.c_str(),
        Internals::icompare_predicate);

    return result.first == &(a.c_str())[a.size()];
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


} // namespace bstone
