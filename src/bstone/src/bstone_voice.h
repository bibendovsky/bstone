/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_VOICE_INCLUDED
#define BSTONE_VOICE_INCLUDED

#include "bstone_audio_mixer_output_gains.h"
#include "bstone_audio_mixer_voice_handle.h"

namespace bstone
{

struct Voice
{
	bool use_output_gains{};
	bstone::AudioMixerVoiceHandle handle;
	double gain{};
	AudioMixerOutputGains output_gains{};
};

} // bstone

#endif // !BSTONE_VOICE_INCLUDED
