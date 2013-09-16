//
// A base class for AdLib decoder.
//


#include "bstone_adlib_decoder.h"

#include <cassert>
#include <cstddef>

#include <algorithm>


namespace bstone {


AdlibDecoder::AdlibDecoder() :
    emulator_()
{
    set_is_initialized(false);
}

// (virtual)
AdlibDecoder::~AdlibDecoder()
{
    uninitialize();
}

// (virtual)
bool AdlibDecoder::initialize(
    const void* raw_data,
    int raw_size,
    int dst_rate)
{
    if (!AudioDecoder::initialize(
        raw_data,
        raw_size,
        dst_rate))
    {
        return false;
    }

    emulator_.initialize(dst_rate);
    set_emulator_default_state();

    return true;
}

// (virtual)
void AdlibDecoder::uninitialize()
{
    emulator_.uninitialize();

    AudioDecoder::uninitialize();
}

// (virtual)
bool AdlibDecoder::reset()
{
    if (!is_initialized())
        return false;

    emulator_.initialize(get_dst_rate());
    set_emulator_default_state();

    return true;
}

void AdlibDecoder::set_emulator_default_state()
{
    for (int i = 1; i <= 0xF5; ++i)
        emulator_.write(i, 0x00);

    emulator_.write(0x01, 0x20);
    emulator_.write(0x08, 0x00);
}


} // namespace bstone

