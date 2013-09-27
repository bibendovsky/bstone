//
// A wrapper to make a value uninitalized.
//


#ifndef BSTONE_UN_VALUE_H
#define BSTONE_UN_VALUE_H


namespace bstone {


// A wrapper to make a value uninitalized.
template<class T>
class UnValue {
public:
    UnValue()
    {
    }

    UnValue(
        const T& that) :
            value_(that)
    {
    }

    operator T&()
    {
        return value_;
    }

private:
    T value_;
}; // class UnValue


} // namespace bstone


#endif // BSTONE_UN_VALUE_H
