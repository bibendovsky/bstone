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

#include "bstone_audio_mixer_utils.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include "3d_def.h"
#include "bstone_math.h"

namespace bstone
{

// ==========================================================================

AudioMixerR3Vector AudioMixerUtils::make_r3_position_from_w3d_coords(double w3d_x, double w3d_y, double w3d_z) noexcept
{
	return AudioMixerR3Vector{w3d_x, w3d_z, w3d_y};
}

AudioMixerListenerR3Orientation AudioMixerUtils::make_listener_r3_orientation_from_w3d_view(double w3d_view_cos, double w3d_view_sin) noexcept
{
	auto result = AudioMixerListenerR3Orientation{};
	result.at = AudioMixerR3Vector{w3d_view_cos, 0.0, -w3d_view_sin};
	result.up = AudioMixerR3Vector{0.0, 1.0, 0.0};
	return result;
}

double AudioMixerUtils::get_r_module(const AudioMixerR3Vector& v) noexcept
{
	return 1.0 / std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

AudioMixerR3Vector AudioMixerUtils::normalize(const AudioMixerR3Vector& v) noexcept
{
	return v * get_r_module(v);
}

double AudioMixerUtils::get_distance(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept
{
	const auto dx = a.x - b.x;
	const auto dy = a.y - b.y;
	const auto dz = a.z - b.z;
	return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
}

double AudioMixerUtils::dot_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept
{
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

AudioMixerR3Vector AudioMixerUtils::cross_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept
{
	return AudioMixerR3Vector{(a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x)};
}

// Notes:
//    - All positions are expected in meters.
//    - Listener's orientation should be normalized.
void AudioMixerUtils::spatialize_voice_2_0(
	AudioMixerDistanceModel distance_model,
	const AudioMixerListenerR3Position& listener_r3_position,
	const AudioMixerListenerR3Orientation& listener_r3_orientation,
	const AudioMixerVoiceR3Attenuation& voice_r3_attenuation,
	const AudioMixerVoiceR3Position& voice_r3_position,
	double& left_gain,
	double& right_gain) noexcept
{
	if (distance_model == AudioMixerDistanceModel::none ||
		voice_r3_attenuation.min_distance == voice_r3_attenuation.max_distance ||
		voice_r3_attenuation.rolloff_factor == 0.0)
	{
		left_gain = 0.5 * audio_mixer_max_gain;
		right_gain = 0.5 * audio_mixer_max_gain;
		return;
	}

	auto distance = get_distance(listener_r3_position, voice_r3_position);
	auto attenuation = 0.0;

	switch (distance_model)
	{
		case AudioMixerDistanceModel::inverse_clamped:
			distance = bstone::math::clamp(distance, voice_r3_attenuation.min_distance, voice_r3_attenuation.max_distance);
			attenuation = voice_r3_attenuation.min_distance / (voice_r3_attenuation.min_distance + (voice_r3_attenuation.rolloff_factor * (distance - voice_r3_attenuation.min_distance)));
			break;

		case AudioMixerDistanceModel::linear_clamped:
			distance = bstone::math::clamp(distance, voice_r3_attenuation.min_distance, voice_r3_attenuation.max_distance);
			attenuation = 1.0 - ((voice_r3_attenuation.rolloff_factor * (distance - voice_r3_attenuation.min_distance)) / (voice_r3_attenuation.max_distance - voice_r3_attenuation.min_distance));
			break;

		default:
			assert(false && "Unknown distance model.");
			left_gain = 0.5 * audio_mixer_max_gain;
			right_gain = 0.5 * audio_mixer_max_gain;
			return;
	}

	attenuation = bstone::math::clamp(attenuation, audio_mixer_min_gain, audio_mixer_max_gain);
	left_gain = 0.5 * attenuation;
	right_gain = 0.5 * attenuation;
	constexpr auto distance_epsilon = 0.000'5;

	if (distance > distance_epsilon)
	{
		const auto voice_direction = normalize(voice_r3_position - listener_r3_position);
		const auto listener_right = cross_product(listener_r3_orientation.at, listener_r3_orientation.up);
		const auto cosine = dot_product(voice_direction, listener_right);
		left_gain *= 1.0 - cosine;
		right_gain *= 1.0 + cosine;
	}
}

} // bstone
