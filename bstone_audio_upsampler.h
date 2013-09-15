//
// An audio upsampler for 16-bit (signed) mono samples.
//


#ifndef BSTONE_AUDIO_UPSAMPLER_H
#define BSTONE_AUDIO_UPSAMPLER_H


#include <cstdint>


namespace bstone {


// An audio upsampler for 16-bit (signed) mono samples.
class AudioUpsampler {
public:
    // Changes a rate of input samples.
    // Returns a number of written samples.
    static int process(
        int src_rate,
        int src_count,
        const int16_t* src_samples,
        int dst_rate,
        int16_t* dst_samples);

    static int calculate_dst_count(
        int src_rate,
        int src_count,
        int dst_rate);

private:
    AudioUpsampler();

    ~AudioUpsampler();

    AudioUpsampler(
        const AudioUpsampler& that);

    AudioUpsampler& operator=(
        const AudioUpsampler& that);
}; // class AudioUpsampler


} // namespace bstone


#endif // BSTONE_AUDIO_UPSAMPLER_H
