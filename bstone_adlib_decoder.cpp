//
// A base class for AdLib decoder.
//


#include "bstone_adlib_decoder.h"

#include <cassert>
#include <cstddef>

#include <algorithm>


namespace bstone {


AdlibDecoder::AdlibDecoder() :
    emulator_(),
    length_in_samples_(0),
    raw_data_(NULL),
    raw_data_size_(0),
    sample_rate_(0)
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
    int raw_data_size,
    int sample_rate)
{
    uninitialize();

    if (raw_data == NULL)
        return false;

    if (raw_data_size < 1)
        return false;

    emulator_.initialize(sample_rate);
    length_in_samples_ = 0;
    set_emulator_default_state();
    raw_data_ = raw_data;
    raw_data_size_ = raw_data_size;
    sample_rate_ = sample_rate;

    return true;
}

// (virtual)
void AdlibDecoder::uninitialize()
{
    emulator_.uninitialize();
    length_in_samples_ = 0;
    set_is_initialized(false);
    raw_data_ = NULL;
    raw_data_size_ = 0;
    sample_rate_ = 0;
}

bool AdlibDecoder::is_initialized() const
{
    return is_initialized_;
}

// (virtual)
bool AdlibDecoder::reset()
{
    if (!is_initialized())
        return false;

    emulator_.initialize(get_sample_rate());
    set_emulator_default_state();

    return true;
}

int AdlibDecoder::get_sample_rate() const
{
    return sample_rate_;
}

int AdlibDecoder::get_length_in_samples() const
{
    return length_in_samples_;
}

const void* AdlibDecoder::get_raw_data() const
{
    return raw_data_;
}

int AdlibDecoder::get_raw_data_size() const
{
    return raw_data_size_;
}

void AdlibDecoder::set_is_initialized(bool value)
{
    is_initialized_ = value;
}

void AdlibDecoder::set_emulator_default_state()
{
    for (int i = 1; i <= 0xF5; ++i)
        emulator_.write(i, 0x00);

    emulator_.write(0x01, 0x20);
    emulator_.write(0x08, 0x00);
}


} // namespace bstone

