//
// A wrapper for C platform-dependent functions.
//


#include "bstone_c.h"

#include <algorithm>
#include <locale>


// FIXME
// Non-"C" locales in MinGW are not supported.


namespace bstone {


// (static)
int C::memicmp(
    const void* buf1,
    const void* buf2,
    size_t cnt)
{
    const char* a = static_cast<const char*>(buf1);
    const char* b = static_cast<const char*>(buf2);

    std::locale locale("");

    const std::ctype<char>& ctype =
        std::use_facet<std::ctype<char> >(locale);

    int diff = 0;

    for (size_t i = 0; diff == 0 && i < cnt; ++i) {
        diff = static_cast<unsigned char>(ctype.tolower(a[i])) -
            static_cast<unsigned char>(ctype.tolower(b[i]));
    }

    return diff;
}

// (static)
int C::stricmp(
    const char* str1,
    const char* str2)
{
    std::locale locale("");

    const std::ctype<char>& ctype =
        std::use_facet<std::ctype<char> >(locale);

    int diff = 0;

    for (size_t i = 0; diff == 0; ++i) {
        char a = str1[i];
        char b = str2[i];

        diff = static_cast<unsigned char>(ctype.tolower(a)) -
            static_cast<unsigned char>(ctype.tolower(b));

        if (a == '\0' || b == '\0')
            break;
    }

    return diff;
}


} // namespace bstone
