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

#include "bstone_audio_mixer.h"

#include <utility>
#include "bstone_oal_audio_mixer.h"
#include "bstone_sdl_audio_mixer.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_memory.h"

namespace bstone
{

namespace
{

class AudioMixerException : public Exception
{
public:
	explicit AudioMixerException(const char* message) noexcept
		:
		Exception{"AUDIO_MIXER", message}
	{
	}
}; // AudioMixerException

[[noreturn]] void audio_mixer_fail(const char* message)
{
	throw AudioMixerException{message};
}

[[noreturn]] void audio_mixer_fail_nested(const char* message)
{
	std::throw_with_nested(AudioMixerException{message});
}

} // namespace

// ==========================================================================

AudioMixerListenerR3Position audio_mixer_make_default_listener_r3_position() noexcept
{
	return AudioMixerListenerR3Position{};
}

AudioMixerR3Vector audio_mixer_make_default_listener_r3_orientation_at() noexcept
{
	return AudioMixerR3Vector{0.0, 0.0, -1.0};
}

AudioMixerR3Vector audio_mixer_make_default_listener_r3_orientation_up() noexcept
{
	return AudioMixerR3Vector{0.0, 1.0, 0.0};
}

AudioMixerListenerR3Orientation audio_mixer_make_default_listener_r3_orientation() noexcept
{
	auto result = AudioMixerListenerR3Orientation{};
	result.at = audio_mixer_make_default_listener_r3_orientation_at();
	result.up = audio_mixer_make_default_listener_r3_orientation_up();
	return result;
}

AudioMixerVoiceR3Position audio_mixer_make_default_voice_r3_position() noexcept
{
	return AudioMixerVoiceR3Position{};
}

AudioMixerVoiceR3Attenuation audio_mixer_make_default_voice_attenuation() noexcept
{
	auto result = AudioMixerVoiceR3Attenuation{};
	result.min_distance = audio_mixer_default_min_distance;
	result.max_distance = audio_mixer_default_max_distance;
	result.roll_off_factor = audio_mixer_default_rolloff_factor;
	return result;
}

// ==========================================================================

AudioMixerUPtr make_audio_mixer(const AudioMixerInitParam& param)
try
{
	switch (param.audio_driver_type)
	{
		case AudioDriverType::r2_sdl:
			return std::make_unique<SdlAudioMixer>(param);

		case AudioDriverType::r3_openal:
			return std::make_unique<OalAudioMixer>(param);

		default:
			throw AudioMixerException{"Unsupported driver type."};
	}
}
catch (...)
{
	audio_mixer_fail_nested(__func__);
}

void AudioMixerListenerR3Position::operator=(const AudioMixerR3Vector& r3_vector) noexcept
{
	x = r3_vector.x;
	y = r3_vector.y;
	z = r3_vector.z;
}

// ==========================================================================

bool operator==(const AudioMixerR3Vector& lhs, const AudioMixerR3Vector& rhs) noexcept
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

bool operator!=(const AudioMixerR3Vector& lhs, const AudioMixerR3Vector& rhs) noexcept
{
	return !(lhs == rhs);
}

// --------------------------------------------------------------------------

bool operator==(const AudioMixerListenerR3Orientation& lhs, const AudioMixerListenerR3Orientation& rhs) noexcept
{
	return lhs.at == rhs.at && lhs.up == rhs.up;
}

bool operator!=(const AudioMixerListenerR3Orientation& lhs, const AudioMixerListenerR3Orientation& rhs) noexcept
{
	return !(lhs == rhs);
}

// --------------------------------------------------------------------------

bool operator==(const AudioMixerVoiceR3Attenuation& lhs, const AudioMixerVoiceR3Attenuation& rhs) noexcept
{
	return lhs.min_distance == rhs.min_distance && lhs.max_distance == rhs.max_distance && lhs.roll_off_factor == rhs.roll_off_factor;
}

bool operator!=(const AudioMixerVoiceR3Attenuation& lhs, const AudioMixerVoiceR3Attenuation& rhs) noexcept
{
	return !(lhs == rhs);
}

// --------------------------------------------------------------------------

AudioMixerR3Vector operator*(const AudioMixerR3Vector& lhs, double rhs) noexcept
{
	return AudioMixerR3Vector{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

AudioMixerVoiceR3Position operator*(const AudioMixerVoiceR3Position& lhs, double rhs) noexcept
{
	return AudioMixerVoiceR3Position{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

AudioMixerListenerR3Position operator*(const AudioMixerListenerR3Position& lhs, double rhs) noexcept
{
	return AudioMixerListenerR3Position{lhs.x * rhs, lhs.y * rhs, lhs.z * rhs};
}

// --------------------------------------------------------------------------

AudioMixerR3Vector operator-(const AudioMixerVoiceR3Position& lhs, const AudioMixerListenerR3Position& rhs) noexcept
{
	return AudioMixerR3Vector{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

} // bstone
