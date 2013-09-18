//
// A PCM decoder for digitized sounds.
//


#ifndef BSTONE_PCM_DECODER_H
#define BSTONE_PCM_DECODER_H


#include "bstone_audio_decoder.h"


namespace bstone {


class PcmDecoder : public AudioDecoder {
public:
    PcmDecoder();

    virtual bool initialize(
        const void* src_raw_data,
        int src_raw_size,
        int dst_rate);

    virtual void uninitialize();

    virtual int decode(
        int dst_count,
        int16_t* dst_data);

    virtual bool reset();

    static int get_src_rate();

    static int get_min_dst_rate();

private:
    int64_t offset_;
    int16_t last_sample_;
    int64_t dst_count_;
    int64_t dst_ratio_;
    double alpha_;
    double one_minus_alpha_;

    static int16_t pcm8_to_pcm16(uint8_t sample);
}; // class PcmDecoder


} // namespace bstone


#endif // BSTONE_PCM_DECODER_H
