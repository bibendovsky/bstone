/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// Audio mixer with system backend

#ifndef BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED
#define BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED

#include "bstone_audio_mixer.h"

namespace bstone {

AudioMixerUPtr make_system_audio_mixer(const AudioMixerInitParam& param);

} // namespace bstone

#endif // BSTONE_SYSTEM_AUDIO_MIXER_INCLUDED
