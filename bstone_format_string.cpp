//
// A wrapper for std::ostringstream to produce a formatted string.
//


#include "bstone_format_string.h"


namespace bstone {


FormatString::FormatString()
{
}

FormatString::operator std::string()
{
    return stream_.str();
}

std::string FormatString::to_string() const
{
    return stream_.str();
}


} //namespace bstone
