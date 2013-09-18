//
// Base class for audio decoding.
//


#ifndef BSTONE_AUDIO_DECODER_H
#define BSTONE_AUDIO_DECODER_H


#include <cstdint>


namespace bstone {


// Base class for audio decoding.
class AudioDecoder {
public:
    AudioDecoder();

    virtual ~AudioDecoder();

    virtual bool initialize(
        const void* src_raw_data,
        int src_raw_size,
        int dst_rate);

    virtual void uninitialize() = 0;

    bool is_initialized() const;

    virtual int decode(
        int dst_count,
        int16_t* dst_data) = 0;

    virtual bool reset() = 0;

    virtual AudioDecoder* clone() = 0;

    int get_dst_rate() const;

    int get_dst_length_in_samples() const;

protected:
    const void* get_raw_data() const;

    int get_raw_size() const;

    void set_is_initialized(
        bool value);

    void set_dst_length_in_samples(
        int value);

private:
    bool is_initialized_;
    const void* raw_data_;
    int raw_size_;
    int dst_rate_;
    int dst_length_in_samples_;
}; // class AudioDecoder


} // namespace bstone


#endif // BSTONE_AUDIO_DECODER_H
