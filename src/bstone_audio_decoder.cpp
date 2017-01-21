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
// Base class for audio decoding.
//


#include "bstone_audio_decoder.h"
#include <cstddef>


namespace bstone {


AudioDecoder::AudioDecoder() :
        is_initialized_(),
        raw_data_(),
        raw_size_(),
        dst_rate_(),
        dst_length_in_samples_()
{
}

// (virtual)
AudioDecoder::~AudioDecoder()
{
}

// (virtual)
bool AudioDecoder::initialize(
    const void* raw_data,
    int raw_size,
    int dst_rate)
{
    uninitialize();

    if (!raw_data) {
        return false;
    }

    if (raw_size < 0) {
        return false;
    }

    if (dst_rate < 1) {
        return false;
    }

    raw_data_ = raw_data;
    raw_size_ = raw_size;
    dst_rate_ = dst_rate;
    dst_length_in_samples_ = 0;

    return true;
}

void AudioDecoder::uninitialize()
{
    dst_length_in_samples_ = 0;
    raw_data_ = nullptr;
    raw_size_ = 0;
    dst_rate_ = 0;
    set_is_initialized(false);
}

bool AudioDecoder::is_initialized() const
{
    return is_initialized_;
}

int AudioDecoder::get_dst_rate() const
{
    return dst_rate_;
}

int AudioDecoder::get_dst_length_in_samples() const
{
    return dst_length_in_samples_;
}

const void* AudioDecoder::get_raw_data() const
{
    return raw_data_;
}

void AudioDecoder::set_is_initialized(
    bool value)
{
    is_initialized_ = value;
}

int AudioDecoder::get_raw_size() const
{
    return raw_size_;
}

void AudioDecoder::set_dst_length_in_samples(
    int value)
{
    dst_length_in_samples_ = value;
}


} // bstone
