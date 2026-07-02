/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Audio decoder interface

#ifndef BSTONE_AUDIO_DECODER_INCLUDED
#define BSTONE_AUDIO_DECODER_INCLUDED

#include "bstone_opl_emulator.h"
#include <memory>

namespace bstone {

inline constexpr int audio_decoder_w3d_pcm_frequency = 7'000;

enum class AudioDecoderType
{
	opl_music = 1,
	opl_sfx = 2,
	pc_speaker = 3,
	pcm = 4,
};

struct AudioDecoderInitParam
{
	const void* src_raw_data;
	int src_raw_size;
	int dst_rate;
};

// =====================================

class AudioDecoder
{
public:
	AudioDecoder() = default;
	virtual ~AudioDecoder() = default;

	virtual bool initialize(const AudioDecoderInitParam& param) = 0;
	virtual void terminate() = 0;
	virtual bool is_initialized() const = 0;
	virtual int get_total_frames() const = 0;
	virtual int get_channel_count() const = 0;
	virtual int decode_frames(float* samples, int frame_count) = 0;
	virtual bool rewind() = 0;
};

// =====================================

using AudioDecoderUPtr = std::unique_ptr<AudioDecoder>;

AudioDecoderUPtr make_audio_decoder(AudioDecoderType audio_decoder_type, OplEmulatorType opl_emulator_type);

} // namespace bstone

#endif // BSTONE_AUDIO_DECODER_INCLUDED
