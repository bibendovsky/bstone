//
// A PCM decoder for digitized sounds.
//


#include "bstone_pcm_decoder.h"

#include <cstddef>


namespace bstone {


PcmDecoder::PcmDecoder() :
    offset_(0),
    last_sample_(0),
    dst_count_(0),
    dst_ratio_(0),
    alpha_(0.0),
    one_minus_alpha_(0.0)
{
}

// (virtual)
bool PcmDecoder::initialize(
    const void* src_raw_data,
    int src_raw_size,
    int dst_rate)
{
    if (!AudioDecoder::initialize(
        src_raw_data,
        src_raw_size,
        dst_rate))
    {
        return false;
    }

    if (dst_rate < get_min_dst_rate())
        return false;

    int64_t i_size = src_raw_size;
    int64_t o_rate = dst_rate;
    int64_t i_rate = get_src_rate();

    set_dst_length_in_samples(
        static_cast<int>((i_size * o_rate) / i_rate));

    dst_count_ = get_dst_length_in_samples();
    dst_ratio_ = (i_size << 16) / dst_count_;

    //
    // Calculate parameters of low-pass filter for a cutoff frequency 'f'.
    //
    // For sampling rate r:
    // dt = 1 / r
    // rc = 1 / 2 * PI * f
    // alpha = dt / (rc + dt)
    //

    const double PI = 3.1415926535897931;

    const double dt = 1.0 / dst_rate;

    // Our cutoff frequency is half of source rate,
    // because it's a maximum frequency allowed by Nyquist.
    const double rc = 1.0 / (PI * get_src_rate());

    alpha_ = dt / (rc + dt);
    one_minus_alpha_ = 1.0 - alpha_;

    set_is_initialized(true);

    return true;
}

// (virtual)
void PcmDecoder::uninitialize()
{
    offset_ = 0;
    last_sample_ = 0;
    dst_count_ = 0;
    dst_ratio_ = 0;
    alpha_ = 0.0;
    one_minus_alpha_ = 0.0;

    AudioDecoder::uninitialize();
}

// (virtual)
int PcmDecoder::decode(
    int dst_count,
    int16_t* dst_data)
{
    if (dst_count <= 0)
        return 0;

    if (dst_data == NULL)
        return 0;

    if (offset_ >= dst_count_)
        return 0;

    int actual_count = 0;
    const uint8_t* src_samples = static_cast<const uint8_t*>(get_raw_data());

    int64_t offset = offset_;

    int cached_index = -1;
    int16_t cached_sample = 0;

    for (int i = 0; i < dst_count && offset < dst_count_; ++i) {
        int src_index = static_cast<int>((offset * dst_ratio_) >> 16);

        if (src_index != cached_index) {
            cached_index = src_index;
            cached_sample = pcm8_to_pcm16(src_samples[src_index]);
        }

        dst_data[i] = cached_sample;

        ++offset;
        ++actual_count;
    }

    offset = offset_;

    for (int i = 0; i < actual_count; ++i) {
        if (!(i == 0 && offset == 0)) {
            int16_t prev_sample;

            if (i > 0)
                prev_sample = dst_data[i - 1];
            else
                prev_sample = last_sample_;

            dst_data[i] = static_cast<int16_t>(
                (alpha_ * dst_data[i]) +
                (one_minus_alpha_ * prev_sample));
        }

        ++offset;
    }

    offset_ = offset;
    last_sample_ = dst_data[actual_count - 1];

    return actual_count;
}

// (virtual)
bool PcmDecoder::reset()
{
    if (!is_initialized())
        return false;

    offset_ = 0;

    return true;
}

// (static)
int PcmDecoder::get_src_rate()
{
    return 7000;
}

// (static)
int PcmDecoder::get_min_dst_rate()
{
    return 11025;
}

// (static)
int16_t PcmDecoder::pcm8_to_pcm16(uint8_t sample)
{
    return static_cast<int16_t>(
        ((static_cast<int>(sample) * 65535) / 255) - 32768);
}


} // namespace bstone
