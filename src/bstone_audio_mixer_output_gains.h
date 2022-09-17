/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED
#define BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED

#include <array>

namespace bstone {

constexpr auto audio_mixer_max_channels = 2;

using AudioMixerOutputGains = std::array<double, audio_mixer_max_channels>;

} // bstone

#endif // !BSTONE_AUDIO_MIXER_OUTPUT_GAINS_INCLUDED
