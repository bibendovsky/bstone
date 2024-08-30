/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED
#define BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED

#include <array>

namespace bstone {

constexpr auto audio_mixer_max_channels = 2;

using AudioMixerOutputGains = std::array<double, audio_mixer_max_channels>;

} // bstone

#endif // !BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED
