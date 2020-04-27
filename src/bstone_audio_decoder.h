/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// Audio decoder interface.
//


#ifndef BSTONE_AUDIO_DECODER_INCLUDED
#define BSTONE_AUDIO_DECODER_INCLUDED


#include <cstdint>

#include <memory>

#include "bstone_opl3.h"


namespace bstone
{


enum class AudioDecoderType
{
	none,
	adlib_music,
	adlib_sfx,
	pcm,
}; // AudioDecoderType


//
// Audio decoder interface.
//
class AudioDecoder
{
public:
	AudioDecoder() = default;

	virtual ~AudioDecoder() = default;


	// Initializes the instance.
	// Returns false on error.
	virtual bool initialize(
		const void* const src_raw_data,
		const int src_raw_size,
		const int dst_rate) = 0;

	// Uninitializes the instance.
	virtual void uninitialize() = 0;

	// Returns true if the instance is initialized or
	// false otherwise.
	virtual bool is_initialized() const noexcept = 0;

	// Decodes specified number of samples into a provided buffer.
	// Returns a number of decoded samples.
	virtual int decode(
		const int dst_count,
		std::int16_t* const dst_data) = 0;

	// Resets the instance.
	virtual bool rewind() = 0;

	// Returns a length of the audio data in samples.
	virtual int get_dst_length_in_samples() const noexcept = 0;
}; // AudioDecoder

using AudioDecoderUPtr = std::unique_ptr<AudioDecoder>;


AudioDecoderUPtr make_audio_decoder(
	const AudioDecoderType audio_decoder_type,
	const Opl3Type opl3_type);


} // bstone


#endif // !BSTONE_AUDIO_DECODER_INCLUDED
