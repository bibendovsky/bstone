/* ==============================================================
bstone: A source port of Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013 Boris Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
============================================================== */


//
// A decoder for AdLib sound effects.
//


#ifndef BSTONE_ADLIB_SFX_DECODER_INCLUDED
#define BSTONE_ADLIB_SFX_DECODER_INCLUDED


#include <cstdint>
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
    }; // Instrument

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
}; // AdlibDecoder


} // namespace bstone


#endif // BSTONE_ADLIB_SFX_DECODER_INCLUDED
