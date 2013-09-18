//
// Base class for audio decoding.
//


#include "bstone_audio_decoder.h"

#include <cstddef>


namespace bstone {


AudioDecoder::AudioDecoder() :
    is_initialized_(false),
    raw_data_(NULL),
    raw_size_(0),
    dst_rate_(0),
    dst_length_in_samples_(0)
{
}

// (virtual)
AudioDecoder::~AudioDecoder()
{
    uninitialize();
}

// (virtual)
bool AudioDecoder::initialize(
    const void* raw_data,
    int raw_size,
    int dst_rate)
{
    uninitialize();

    if (raw_data == NULL)
        return false;

    if (raw_size < 0)
        return false;

    if (dst_rate < 1)
        return false;

    raw_data_ = raw_data;
    raw_size_ = raw_size;
    dst_rate_ = dst_rate;
    dst_length_in_samples_ = 0;

    return true;
}

void AudioDecoder::uninitialize()
{
    dst_length_in_samples_ = 0;
    raw_data_ = NULL;
    raw_size_ = 0;
    dst_rate_ = 0;
    set_is_initialized(false);
}

bool AudioDecoder::is_initialized() const
{
    return is_initialized_;
}

int AudioDecoder::get_dst_rate() const
{
    return dst_rate_;
}

int AudioDecoder::get_dst_length_in_samples() const
{
    return dst_length_in_samples_;
}

const void* AudioDecoder::get_raw_data() const
{
    return raw_data_;
}

void AudioDecoder::set_is_initialized(
    bool value)
{
    is_initialized_ = value;
}

int AudioDecoder::get_raw_size() const
{
    return raw_size_;
}

void AudioDecoder::set_dst_length_in_samples(
    int value)
{
    dst_length_in_samples_ = value;
}


} // namespace bstone
