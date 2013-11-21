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
