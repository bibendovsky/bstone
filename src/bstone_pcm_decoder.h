/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
*/


//
// A PCM decoder for digitized sounds.
//


#ifndef BSTONE_PCM_DECODER_INCLUDED
#define BSTONE_PCM_DECODER_INCLUDED


#include "bstone_audio_decoder.h"


namespace bstone {


// A PCM decoder for digitized sounds.
class PcmDecoder : public AudioDecoder {
public:
    PcmDecoder();

    virtual ~PcmDecoder();

    virtual bool initialize(
        const void* src_raw_data,
        int src_raw_size,
        int dst_rate);

    virtual void uninitialize();

    virtual int decode(
        int dst_count,
        int16_t* dst_data);

    virtual bool reset();

    virtual AudioDecoder* clone();

    // Return an input sample rate.
    static int get_src_rate();

    // Returns a minimum output sample rate.
    static int get_min_dst_rate();

private:
    int64_t offset_;
    int16_t last_sample_;
    int64_t dst_count_;
    int64_t dst_ratio_;
    double alpha_;
    double one_minus_alpha_;

    static int16_t pcm8_to_pcm16(
        uint8_t sample);
}; // PcmDecoder


} // bstone


#endif // BSTONE_PCM_DECODER_INCLUDED
