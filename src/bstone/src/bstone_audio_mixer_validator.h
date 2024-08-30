/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED
#define BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED

#include "bstone_audio_mixer.h"

namespace bstone
{

class AudioMixerValidator
{
public:
	static void validate_gain(double gain);
	static void validate_output_gains(const AudioMixerOutputGains& output_gains);
}; // AudioMixerValidator

} // bstone

#endif // !BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED
