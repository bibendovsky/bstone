#ifndef BSTONE_STRING_HELPER_H
#define BSTONE_STRING_HELPER_H


#include <string>
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
