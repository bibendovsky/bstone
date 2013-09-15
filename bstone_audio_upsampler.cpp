//
// An audio upsampler for 16-bit (signed) mono samples.
//


#include "bstone_audio_upsampler.h"

#include <cstddef>


namespace bstone {


// (static)
int AudioUpsampler::process(
    int src_rate,
    int src_count,
    const int16_t* src_samples,
    int dst_rate,
    int16_t* dst_samples)
{
    if (src_rate <= 0)
        return 0;

    if (src_count <= 0)
        return 0;

    if (src_samples == NULL)
        return 0;

    if (dst_rate <= src_rate)
        return 0;

    if (dst_samples == NULL)
        return 0;

    if (src_count == 1) {
        dst_samples[0] = src_samples[0];
        return 1;
    }

    int64_t i_count = src_count;

    int o_count = calculate_dst_count(
        src_rate,
        src_count,
        dst_rate);

    int64_t o_ratio = (i_count << 16) / o_count;

    for (int i = 0; i < o_count; ++i) {
        int src_index = static_cast<int>((i * o_ratio) >> 16);

        dst_samples[i] = src_samples[src_index];
    }


    //
    // Apply low-pass filter for a cutoff frequency 'f'.
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
    const double rc = 1.0 / (PI * src_rate);

    const double alpha = dt / (rc + dt);
    const double one_minus_alpha = 1.0 - alpha;

    for (int i = 1; i < o_count; ++i) {
        dst_samples[i] = static_cast<int16_t>((alpha * dst_samples[i]) +
            (one_minus_alpha * dst_samples[i - 1]));
    }

    return static_cast<int>(o_count);
}

// (static)
int AudioUpsampler::calculate_dst_count(
    int src_rate,
    int src_count,
    int dst_rate)
{
    if (src_rate <= 0)
        return 0;

    if (src_count <= 0)
        return 0;

    if (dst_rate <= 0)
        return 0;

    if (src_rate >= dst_rate)
        return 0;

    int64_t i_rate = src_rate;
    int64_t i_count = src_count;
    int64_t o_rate = dst_rate;

    return static_cast<int>((i_count * o_rate) / i_rate);
}


} // namespace bstone
