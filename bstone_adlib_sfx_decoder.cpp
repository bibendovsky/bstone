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
// A decoder for AdLib sound effects.
//


#include "bstone_adlib_sfx_decoder.h"
#include <cstdint>
#include "bstone_endian.h"


namespace bstone {


void AdlibSfxDecoder::Instrument::reset()
{
    m_char = 0;
    c_char = 0;
    m_scale = 0;
    c_scale = 0;
    m_attack = 0;
    c_attack = 0;
    m_sus = 0;
    c_sus = 0;
    m_wave = 0;
    c_wave = 0;
}

AdlibSfxDecoder::AdlibSfxDecoder() :
    commands_count_(0),
    command_index_(0),
    samples_per_tick_(0),
    remains_count_(0),
    hf_(0)
{
}

// (virtual)
AdlibSfxDecoder::~AdlibSfxDecoder()
{
    uninitialize();
}

// (virtual)
bool AdlibSfxDecoder::initialize(
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

    int sfx_length = bstone::Endian::le(reader_.read_s32());

    if (sfx_length <= 0)
        return false;

    if ((sfx_length + get_header_size()) >= raw_size)
        return false;

    // Skip priority.
    reader_.skip(2);

    instrument_.m_char = reader_.read_u8();
    instrument_.c_char = reader_.read_u8();
    instrument_.m_scale = reader_.read_u8();
    instrument_.c_scale = reader_.read_u8();
    instrument_.m_attack = reader_.read_u8();
    instrument_.c_attack = reader_.read_u8();
    instrument_.m_sus = reader_.read_u8();
    instrument_.c_sus = reader_.read_u8();
    instrument_.m_wave = reader_.read_u8();
    instrument_.c_wave = reader_.read_u8();

    // Skip nConn, voice, mode and 3 unused octets
    reader_.skip(6);

    if (instrument_.m_sus == 0 && instrument_.c_sus == 0)
        return false;

    hf_ = reader_.read_u8();
    hf_ = ((hf_ & 7) << 2) | 0x20;

    initialize_instrument();

    command_index_ = 0;
    commands_count_ = sfx_length;
    samples_per_tick_ = emulator_.get_sample_rate() / get_tick_rate();
    set_dst_length_in_samples(samples_per_tick_ * sfx_length);
    remains_count_ = 0;

    set_is_initialized(true);

    return true;
}

// (virtual)
void AdlibSfxDecoder::uninitialize()
{
    reader_.close();
    instrument_.reset();
    commands_count_ = 0;
    command_index_ = 0;
    samples_per_tick_ = 0;
    remains_count_ = 0;
    hf_ = 0;

    AdlibDecoder::uninitialize();
}

// (virtual)
bool AdlibSfxDecoder::reset()
{
    if (!AdlibDecoder::reset())
        return false;

    initialize_instrument();

    command_index_ = 0;
    remains_count_ = 0;

    reader_.set_position(get_header_size());

    return true;
}

// (virtual)
AudioDecoder* AdlibSfxDecoder::clone()
{
    return new AdlibSfxDecoder(*this);
}

// (virtual)
int AdlibSfxDecoder::decode(
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

    for (bool quit = false; !quit; ) {
        if (remains_count_ > 0) {
            int count = std::min(dst_count, remains_count_);

            emulator_.generate(count, dst_data);

            dst_data += count;
            dst_count -= count;
            remains_count_ -= count;
            decoded_samples_count += count;
        } else {
            if (command_index_ < commands_count_) {
                int lf = reader_.read_u8();

                if (lf > 0) {
                    emulator_.write(AL_FREQ_L, lf);
                    emulator_.write(AL_FREQ_H, hf_);
                } else
                    emulator_.write(AL_FREQ_H, 0x00);

                ++command_index_;

                remains_count_ = samples_per_tick_;
            }
        }

        quit =
            (command_index_ == commands_count_ && remains_count_ == 0) ||
            dst_count == 0;
    }

    return decoded_samples_count;
}

void AdlibSfxDecoder::initialize_instrument()
{
    // carrier
    const int c = 3;

    // modifier
    const int m = 0;

    emulator_.write(m + AL_CHAR, instrument_.m_char);
    emulator_.write(m + AL_SCALE, instrument_.m_scale);
    emulator_.write(m + AL_ATTACK, instrument_.m_attack);
    emulator_.write(m + AL_SUS, instrument_.m_sus);
    emulator_.write(m + AL_WAVE, instrument_.m_wave);
    emulator_.write(c + AL_CHAR, instrument_.c_char);
    emulator_.write(c + AL_SCALE, instrument_.c_scale);
    emulator_.write(c + AL_ATTACK, instrument_.c_attack);
    emulator_.write(c + AL_SUS, instrument_.c_sus);
    emulator_.write(c + AL_WAVE, instrument_.c_wave);

    // AL_FEED_CON
    emulator_.write(AL_FEED_CON, 0);
}

// (static)
int AdlibSfxDecoder::get_tick_rate()
{
    return 140;
}

// (static)
int AdlibSfxDecoder::get_header_size()
{
    return 23;
}


} // namespace bstone
