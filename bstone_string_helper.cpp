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


} // namespace bstone
