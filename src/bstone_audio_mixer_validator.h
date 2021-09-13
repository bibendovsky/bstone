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

#ifndef BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED
#define BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED

#include "bstone_audio_mixer.h"

namespace bstone
{

class AudioMixerValidator
{
public:
	static void validate_distance_model(AudioMixerDistanceModel distance_model);
	static void validate_listener_meters_per_unit(double meters_per_unit);
	static void validate_gain(double gain);
	static void validate_voice_r3_attenuation(const AudioMixerVoiceR3Attenuation& voice_r3_attenuation);

private:
	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
}; // AudioMixerValidator

} // bstone

#endif // !BSTONE_AUDIO_MIXER_VALIDATOR_INCLUDED
