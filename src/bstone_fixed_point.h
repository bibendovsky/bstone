//
// Fixed-point number.
//


#ifndef BSTONE_FIXED_POINT_INCLUDED
#define BSTONE_FIXED_POINT_INCLUDED


#include <cstdint>


namespace bstone
{


class FixedPoint
{
public:
    static constexpr int frac_bits = 16;
    static constexpr int max_frac = 1 << frac_bits;
    static constexpr int frac_mask = max_frac - 1;


    using Value = int;


    FixedPoint(
        Value new_value = 0) :
            value{new_value}
    {
    }

    FixedPoint(
        int int_part,
        int frac_part) :
            value{(int_part << frac_bits) | (frac_part & frac_mask)}
    {
    }


    int get_int() const
    {
        return value >> frac_bits;
    }

    int get_frac() const
    {
        return value & frac_mask;
    }


    Value value;
}; // FixedPoint


} // bstone


bstone::FixedPoint operator+(
    const bstone::FixedPoint& a,
    const bstone::FixedPoint& b)
{
    return a.value + b.value;
}

bstone::FixedPoint& operator+=(
    bstone::FixedPoint& a,
    const bstone::FixedPoint& b)
{
    a.value += b.value;
    return a;
}

bstone::FixedPoint operator/(
    const bstone::FixedPoint& a,
    const int b)
{
    return a.value / b;
}

bstone::FixedPoint operator*(
    const bstone::FixedPoint& a,
    const int b)
{
    return a.value * b;
}

bstone::FixedPoint operator*(
    const int a,
    const bstone::FixedPoint& b)
{
    return a * b.value;
}


#endif // BSTONE_FIXED_POINT_INCLUDED
