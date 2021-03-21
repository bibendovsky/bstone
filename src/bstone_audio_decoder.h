/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

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


constexpr auto audio_decoder_pcm_fixed_frequency = 7'000;


enum class AudioDecoderType
{
	adlib_music = 1,
	adlib_sfx = 2,
	pcm = 3,
	pc_speaker = 4,
}; // AudioDecoderType

enum class AudioDecoderInterpolationType
{
	none,
	zoh,
	linear,
}; // AudioDecoderInterpolationType


struct AudioDecoderInitParam
{
	const void* src_raw_data_;
	int src_raw_size_;
	int dst_rate_;
	AudioDecoderInterpolationType resampler_interpolation_;
	bool resampler_lpf_;
}; // AudioDecoderInitParam


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
		const AudioDecoderInitParam& param) = 0;

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

	// Decodes specified number of samples into a provided buffer.
	// Returns a number of decoded samples.
	virtual int decode(
		const int dst_count,
		float* const dst_data) = 0;

	// Sets decoding position to the beginning.
	virtual bool rewind() = 0;

	// Returns a length of the audio data in samples.
	virtual int get_dst_length_in_samples() const noexcept = 0;

	virtual bool set_resampling(
		const AudioDecoderInterpolationType interpolation_type,
		const bool lpf,
		const bool lpf_flush_samples) = 0;
}; // AudioDecoder


using AudioDecoderUPtr = std::unique_ptr<AudioDecoder>;


AudioDecoderUPtr make_audio_decoder(
	const AudioDecoderType audio_decoder_type,
	const Opl3Type opl3_type);


} // bstone


#endif // !BSTONE_AUDIO_DECODER_INCLUDED
