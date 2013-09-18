//
// A base class for AdLib audio decoding.
//


#ifndef BSTONE_ADLIB_DECODER_H
#define BSTONE_ADLIB_DECODER_H


#include "bstone_audio_decoder.h"
#include "bstone_opl2.h"


namespace bstone {


// A base class for AdLib audio decoding.
class AdlibDecoder : public AudioDecoder {
public:
    AdlibDecoder();

    virtual bool initialize(
        const void* raw_data,
        int raw_size,
        int dst_rate);

    virtual void uninitialize();

    virtual bool reset();

protected:
    Opl2 emulator_;

    void set_emulator_default_state();
}; // class AdlibDecoder


} // namespace bstone


#endif // BSTONE_ADLIB_DECODER_H
