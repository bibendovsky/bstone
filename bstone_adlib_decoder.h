//
// A base class for AdLib decoder.
//


#ifndef BSTONE_ADLIB_DECODER_H
#define BSTONE_ADLIB_DECODER_H


#include "bstone_opl2.h"


namespace bstone {


// A base class for decoder.
class AdlibDecoder {
public:
    AdlibDecoder();

    virtual ~AdlibDecoder();

    virtual bool initialize(
        const void* raw_data,
        int raw_data_size,
        int sample_rate);

    virtual void uninitialize();

    bool is_initialized() const;

    virtual bool reset();

    virtual int decode(
        int samples_count,
        int16_t* buffer) = 0;

    int get_sample_rate() const;

    virtual int get_length_in_samples() const;

protected:
    Opl2 emulator_;
    int length_in_samples_;

    const void* get_raw_data() const;

    int get_raw_data_size() const;

    void set_is_initialized(
        bool value);

    void set_emulator_default_state();

private:
    bool is_initialized_;
    const void* raw_data_;
    int raw_data_size_;
    int sample_rate_;

    AdlibDecoder(
        const AdlibDecoder& that);

    AdlibDecoder& operator=(
        const AdlibDecoder& that);
}; // class AdlibDecoder


} // namespace bstone


#endif // BSTONE_ADLIB_DECODER_H
