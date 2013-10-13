//
// A wrapper for std::ostringstream to produce a formatted string.
//


#ifndef BSTONE_FORMAT_STRING_H
#define BSTONE_FORMAT_STRING_H


#include <iomanip>
#include <sstream>


namespace bstone {


// A wrapper for std::ostringstream to produce a formatted string.
class FormatString {
public:
    FormatString();

    operator std::string();

    std::string to_string() const;

    template<class T>
    bstone::FormatString& operator <<(
        const T& value)
    {
        stream_ << value;
        return *this;
    }

private:
    std::ostringstream stream_;

    FormatString(
        const FormatString& that);

    FormatString& operator=(
        const FormatString& that);
}; // class FormatString


} //namespace bstone


#endif // BSTONE_FORMAT_STRING_H
