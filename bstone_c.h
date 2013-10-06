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
        if (str == NULL)
            return NULL;

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
}; // class C


} // namespace bstone


#endif // BSTONE_C_H

