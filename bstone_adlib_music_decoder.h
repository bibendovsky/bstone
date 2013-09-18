//
// A decoder for AdLib music.
//


#ifndef BSTONE_ADLIB_MUSIC_DECODER_H
#define BSTONE_ADLIB_MUSIC_DECODER_H


#include <cstdint>

#include <vector>

#include "bstone_adlib_decoder.h"
#include "bstone_memory_binary_reader.h"


namespace bstone {


// A decoder for AdLib music.
class AdlibMusicDecoder : public AdlibDecoder {
public:
    AdlibMusicDecoder();

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
    MemoryBinaryReader reader_;
    int commands_count_;
    int command_index_;
    int samples_per_tick_;
    int remains_count_;
}; // class AdlibDecoder


} // namespace bstone


#endif // BSTONE_ADLIB_MUSIC_DECODER_H
