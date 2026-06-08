/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_OAL_AUDIO_MIXER_INCLUDED
#define BSTONE_OAL_AUDIO_MIXER_INCLUDED

#include "bstone_audio_mixer.h"

namespace bstone {

AudioMixerUPtr make_oal_audio_mixer(const AudioMixerInitParam& param);

} // namespace bstone

#endif // BSTONE_OAL_AUDIO_MIXER_INCLUDED
