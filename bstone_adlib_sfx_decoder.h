//
// A decoder for AdLib sound effects.
//


#ifndef BSTONE_ADLIB_SFX_DECODER_H
#define BSTONE_ADLIB_SFX_DECODER_H


#include <stdint.h>

#include <vector>

#include "bstone_adlib_decoder.h"
#include "bstone_memory_binary_reader.h"

namespace bstone {


// A decoder for AdLib sound effects.
class AdlibSfxDecoder : public AdlibDecoder {
public:
    AdlibSfxDecoder();

    virtual ~AdlibSfxDecoder();

    virtual bool initialize(
        const void* raw_data,
        int raw_size,
        int dst_rate);

    virtual void uninitialize();

    virtual bool reset();

    virtual AudioDecoder* clone();

    virtual int decode(
        int dst_count,
        int16_t* dst_data);

    // Returns a number of calls per second of
    // original interrupt routine.
    static int get_tick_rate();

private:
    static const int AL_CHAR = 0x20;
    static const int AL_SCALE = 0x40;
    static const int AL_ATTACK = 0x60;
    static const int AL_SUS = 0x80;
    static const int AL_WAVE = 0xE0;
    static const int AL_FREQ_L = 0xA0;
    static const int AL_FREQ_H = 0xB0;
    static const int AL_FEED_CON = 0xC0;

    class Instrument {
    public:
        int m_char;
        int c_char;
        int m_scale;
        int c_scale;
        int m_attack;
        int c_attack;
        int m_sus;
        int c_sus;
        int m_wave;
        int c_wave;

        void reset();
    }; // class Instrument

    MemoryBinaryReader reader_;
    Instrument instrument_;
    int commands_count_;
    int command_index_;
    int samples_per_tick_;
    int remains_count_;
    int hf_;

    void initialize_instrument();

    // Returns an original size of an AdLibSound structure.
    static int get_header_size();
}; // class AdlibDecoder


} // namespace bstone


#endif // BSTONE_ADLIB_SFX_DECODER_H
