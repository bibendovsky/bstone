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


#ifndef BSTONE_STRING_HELPER_H
#define BSTONE_STRING_HELPER_H


#include <locale>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>


namespace bstone {


typedef std::string::iterator StringIt;
typedef std::string::const_iterator StringCIt;

typedef std::vector<std::string> StringList;
typedef StringList::iterator StringListIt;
typedef StringList::const_iterator StringListCIt;


class StringHelper {
public:
    static char to_lower(
        char value);

    static std::string to_lower(
        const std::string& value);

    template<typename T,typename U>
    static T lexical_cast(
        const U& src_value)
    {
        std::stringstream oss;
        oss << src_value;

        T result;
        oss >> result;

        if (oss)
            return result;

        throw std::runtime_error("lexical_cast");
    }

    template<typename T,typename U>
    static bool lexical_cast(
        const T& src_value,
        U& dst_value)
    {
        std::stringstream oss;
        oss.unsetf(std::ios_base::skipws);
        oss << src_value;
        oss >> dst_value;
        return !oss.fail();
    }

    static bool is_iequal(
        const std::string& a,
        const std::string& b);

    static bool is(
        std::ctype_base::mask mask,
        char value);

    static const std::string& get_empty();

private:
    StringHelper();

    StringHelper(
        const StringHelper& that);

    ~StringHelper();

    StringHelper& operator=(
        const StringHelper& that);
}; // StringHelper


} // namespace bstone


#endif // BSTONE_STRING_HELPER_H
