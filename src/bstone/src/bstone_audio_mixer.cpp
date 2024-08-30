/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <stdexcept>
#include <utility>
#include "bstone_audio_mixer.h"
#include "bstone_exception.h"
#include "bstone_logger.h"
#include "bstone_memory.h"
#include "bstone_oal_audio_mixer.h"
#include "bstone_system_audio_mixer.h"

namespace bstone
{

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

// ==========================================================================

AudioMixer::AudioMixer() noexcept = default;

AudioMixer::~AudioMixer() = default;

// ==========================================================================

AudioMixerUPtr make_audio_mixer(const AudioMixerInitParam& param)
try {
	switch (param.audio_driver_type)
	{
		case AudioDriverType::system:
			return std::make_unique<SystemAudioMixer>(param);

		case AudioDriverType::openal:
			return std::make_unique<OalAudioMixer>(param);

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported driver type.");
	}
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

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
