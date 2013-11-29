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
// Base class for audio decoding.
//


#ifndef BSTONE_AUDIO_DECODER_H
#define BSTONE_AUDIO_DECODER_H


#include <stdint.h>


namespace bstone {


// Base class for audio decoding.
class AudioDecoder {
public:
    AudioDecoder();

    virtual ~AudioDecoder();

    // Initializes the instance.
    // Returns false on error.
    virtual bool initialize(
        const void* src_raw_data,
        int src_raw_size,
        int dst_rate);

    // Uninitializes the instance.
    virtual void uninitialize();

    // Returns true if the instance is initialized or
    // false otherwise.
    bool is_initialized() const;

    // Decodes specified number of samples into a provided buffer.
    // Returns a number of decoded samples.
    virtual int decode(
        int dst_count,
        int16_t* dst_data) = 0;

    // Resets the instance.
    virtual bool reset() = 0;

    // Creates a clone of the instance.
    virtual AudioDecoder* clone() = 0;

    // Returns an output sample rate.
    int get_dst_rate() const;

    // Returns a length of the audio data in samples.
    int get_dst_length_in_samples() const;

protected:
    const void* get_raw_data() const;

    int get_raw_size() const;

    void set_is_initialized(
        bool value);

    void set_dst_length_in_samples(
        int value);

private:
    bool is_initialized_;
    const void* raw_data_;
    int raw_size_;
    int dst_rate_;
    int dst_length_in_samples_;
}; // class AudioDecoder


} // namespace bstone


#endif // BSTONE_AUDIO_DECODER_H
