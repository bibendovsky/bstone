/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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

constexpr auto audio_decoder_w3d_pcm_frequency = 7'000;

enum class AudioDecoderType
{
	adlib_music = 1,
	adlib_sfx = 2,
	pc_speaker = 3,
	pcm = 4,
}; // AudioDecoderType

struct AudioDecoderInitParam
{
	const void* src_raw_data_;
	int src_raw_size_;
	int dst_rate_;
}; // AudioDecoderInitParam

// ==========================================================================

//
// Audio decoder interface.
//
class AudioDecoder
{
public:
	AudioDecoder() noexcept;
	virtual ~AudioDecoder();

	// Initializes the instance.
	// Returns false on error.
	virtual bool initialize(const AudioDecoderInitParam& param) = 0;

	// Uninitializes the instance.
	virtual void uninitialize() = 0;

	// Returns true if the instance is initialized or
	// false otherwise.
	virtual bool is_initialized() const noexcept = 0;

	// Decodes specified number of samples into a provided buffer.
	// Returns a number of decoded samples.
	virtual int decode(int dst_count, std::int16_t* dst_data) = 0;

	// Sets decoding position to the beginning.
	virtual bool rewind() = 0;

	// Returns a length of the audio data in samples.
	virtual int get_dst_length_in_samples() const noexcept = 0;
}; // AudioDecoder

// ==========================================================================

using AudioDecoderUPtr = std::unique_ptr<AudioDecoder>;

AudioDecoderUPtr make_audio_decoder(const AudioDecoderType audio_decoder_type, const Opl3Type opl3_type);

} // bstone

#endif // !BSTONE_AUDIO_DECODER_INCLUDED
