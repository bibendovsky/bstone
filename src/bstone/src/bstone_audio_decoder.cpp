/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// Audio decoder interface.
//

#include "bstone_audio_decoder.h"

#include "bstone_assert.h"
#include "bstone_pc_speaker_audio_decoder.h"
#include "bstone_pcm_audio_decoder.h"
#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"

namespace bstone
{

AudioDecoder::AudioDecoder() noexcept = default;

AudioDecoder::~AudioDecoder() = default;

AudioDecoderUPtr make_audio_decoder(const AudioDecoderType audio_decoder_type, const Opl3Type opl3_type)
{
	switch (audio_decoder_type)
	{
		case AudioDecoderType::adlib_music:
			return make_adlib_music_audio_decoder(opl3_type);

		case AudioDecoderType::adlib_sfx:
			return make_adlib_sfx_audio_decoder(opl3_type);

		case AudioDecoderType::pc_speaker:
			return make_pc_speaker_audio_decoder();

		case AudioDecoderType::pcm:
			return make_pcm_audio_decoder();

		default:
			BSTONE_ASSERT(false && "Unknown audio decoder type.");
			return nullptr;
	}
}

} // bstone
