/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_VOICE_GROUP_INCLUDED
#define BSTONE_VOICE_GROUP_INCLUDED

#include "bstone_audio_mixer.h"
#include "bstone_voice.h"
#include <memory>

namespace bstone {

class VoiceGroup
{
public:
	VoiceGroup() = default;
	virtual ~VoiceGroup() = default;

	virtual bool is_any_playing() = 0;
	virtual void set_gain(double gain) = 0;
	virtual void pause() = 0;
	virtual void resume() = 0;
	virtual void stop() = 0;

	virtual void add_voice(Voice& voice) = 0;
	virtual void stop_and_remove_voice(Voice& voice) = 0;
	virtual void stop_voice(Voice& voice) = 0;
	virtual void set_voice_gain(const Voice& voice) = 0;
	virtual void set_voice_output_gains(const Voice& voice) = 0;
};

// ==========================================================================

using VoiceGroupUPtr = std::unique_ptr<VoiceGroup>;

VoiceGroupUPtr make_voice_group(AudioMixer& audio_mixer);

} // bstone

#endif // !BSTONE_VOICE_GROUP_INCLUDED
