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

#include "bstone_audio_mixer_validator.h"
#include "bstone_exception.h"

namespace bstone
{

namespace
{

class AudioMixerValidatorException : public Exception
{
public:
	explicit AudioMixerValidatorException(const char* message) noexcept
		:
		Exception{"AUDIO_MIXER_VALIDATOR", message}
	{
	}
}; // AudioMixerValidatorException

} // namespace

// ==========================================================================

void AudioMixerValidator::validate_distance_model(AudioMixerDistanceModel distance_model)
try
{
	switch (distance_model)
	{
		case AudioMixerDistanceModel::inverse_clamped:
		case AudioMixerDistanceModel::linear_clamped:
			break;

		default:
			fail("Unknown distance model.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void AudioMixerValidator::validate_listener_meters_per_unit(double meters_per_unit)
try
{
	if (meters_per_unit <= 0.0)
	{
		fail("Meters per unit out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void AudioMixerValidator::validate_gain(double gain)
try
{
	if (gain < audio_mixer_min_gain || gain > audio_mixer_max_gain)
	{
		fail("Gain out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

void AudioMixerValidator::validate_voice_r3_attenuation(const AudioMixerVoiceR3Attenuation& r3_attenuation)
try
{
	if (r3_attenuation.min_distance < audio_mixer_min_min_distance)
	{
		fail("Min distance out of range.");
	}

	if (r3_attenuation.max_distance < audio_mixer_min_max_distance)
	{
		fail("Max distance out of range.");
	}

	if (r3_attenuation.max_distance < r3_attenuation.min_distance)
	{
		fail("Max distance less than the min one.");
	}

	if (r3_attenuation.roll_off_factor < audio_mixer_min_rolloff_factor)
	{
		fail("Rolloff factor out of range.");
	}
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void AudioMixerValidator::fail(const char* message)
{
	throw AudioMixerValidatorException{message};
}

[[noreturn]] void AudioMixerValidator::fail_nested(const char* message)
{
	std::throw_with_nested(AudioMixerValidatorException{message});
}

} // bstone
