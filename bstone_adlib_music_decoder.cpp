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
// A decoder for AdLib music.
//


#include "bstone_adlib_music_decoder.h"
#include <cstdint>
#include "bstone_endian.h"


namespace bstone {


AdlibMusicDecoder::AdlibMusicDecoder() :
    commands_count_(0),
    command_index_(0),
    samples_per_tick_(0),
    remains_count_(0)
{
}

// (virtual)
AdlibMusicDecoder::~AdlibMusicDecoder()
{
    uninitialize();
}

// (virtual)
bool AdlibMusicDecoder::initialize(
    const void* raw_data,
    int raw_size,
    int dst_rate)
{
    if (!AdlibDecoder::initialize(
        raw_data,
        raw_size,
        dst_rate))
    {
        return false;
    }

    reader_.open(raw_data, raw_size);

    int commands_size = bstone::Endian::le(reader_.read_u16());

    if ((commands_size % 4) != 0)
        return false;

    if ((commands_size + 2) > raw_size)
        return false;

    command_index_ = 0;
    commands_count_ = commands_size / 4;

    samples_per_tick_ =  emulator_.get_sample_rate() / get_tick_rate();
    remains_count_ = 0;

    int ticks_count = 0;

    for (int i = 0; i < commands_count_; ++i) {
        reader_.skip(2);
        ticks_count += bstone::Endian::le(reader_.read_u16());
    }

    set_dst_length_in_samples(ticks_count * samples_per_tick_);

    reader_.set_position(2);

    set_is_initialized(true);

    return true;
}

// (virtual)
void AdlibMusicDecoder::uninitialize()
{
    reader_.close();
    commands_count_ = 0;
    command_index_ = 0;
    samples_per_tick_ = 0;
    remains_count_ = 0;

    AdlibDecoder::uninitialize();
}

// (virtual)
bool AdlibMusicDecoder::reset()
{
    if (!AdlibDecoder::reset())
        return false;

    reader_.set_position(2);

    command_index_ = 0;
    remains_count_ = 0;

    return true;
}

// (virtual)
AudioDecoder* AdlibMusicDecoder::clone()
{
    return new AdlibMusicDecoder(*this);
}

// (virtual)
int AdlibMusicDecoder::decode(
    int dst_count,
    int16_t* dst_data)
{
    if (!is_initialized())
        return 0;

    if (dst_count < 1)
        return 0;

    if (!dst_data)
        return 0;

    if (command_index_ == commands_count_ && remains_count_ == 0)
        return 0;

    int decoded_samples_count = 0;

    for (bool quit = false; !quit; )
    {
        if (remains_count_ > 0) {
            int count = std::min(dst_count, remains_count_);

            emulator_.generate(count, dst_data);

            dst_data += count;
            dst_count -= count;
            remains_count_ -= count;
            decoded_samples_count += count;
        } else {
            int delay = 0;

            while (command_index_ < commands_count_ && delay == 0) {
                int command_port = reader_.read_u8();
                int command_value = reader_.read_u8();
                delay = bstone::Endian::le(reader_.read_u16());

                emulator_.write(command_port, command_value);
                ++command_index_;
            }

            if (delay > 0)
                remains_count_ = delay * samples_per_tick_;
        }

        quit =
            (command_index_ == commands_count_ && remains_count_ == 0) ||
            dst_count == 0;
    }

    return decoded_samples_count;
}

// (static)
int AdlibMusicDecoder::get_tick_rate()
{
    return 700;
}


} // namespace bstone
