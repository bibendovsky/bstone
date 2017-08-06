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
// A decoder for AdLib music.
//


#ifndef BSTONE_ADLIB_MUSIC_DECODER_INCLUDED
#define BSTONE_ADLIB_MUSIC_DECODER_INCLUDED


#include <cstdint>
#include <vector>
#include "bstone_adlib_decoder.h"
#include "bstone_memory_binary_reader.h"


namespace bstone {


// A decoder for AdLib music.
class AdlibMusicDecoder : public AdlibDecoder {
public:
    AdlibMusicDecoder();

    virtual ~AdlibMusicDecoder();

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
}; // AdlibDecoder


} // bstone


#endif // BSTONE_ADLIB_MUSIC_DECODER_INCLUDED
