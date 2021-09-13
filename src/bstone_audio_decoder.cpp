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

#include "bstone_audio_decoder.h"
#include <cassert>
#include "bstone_pc_speaker_audio_decoder.h"
#include "bstone_pcm_audio_decoder.h"
#include "bstone_adlib_music_decoder.h"
#include "bstone_adlib_sfx_decoder.h"

namespace bstone
{

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
			assert(false && "Unknown audio decoder type.");
			return nullptr;
	}
}

} // bstone
