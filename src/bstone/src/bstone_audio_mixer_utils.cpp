/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
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
	const AudioMixerListenerR3Position& listener_r3_position,
	const AudioMixerListenerR3Orientation& listener_r3_orientation,
	const AudioMixerVoiceR3Position& voice_r3_position,
	double& left_gain,
	double& right_gain) noexcept
{
	constexpr auto distance_epsilon = 0.000'5;
	const auto distance = get_distance(listener_r3_position, voice_r3_position);

	if (distance > distance_epsilon)
	{
		const auto voice_direction = normalize(voice_r3_position - listener_r3_position);
		const auto listener_right = cross_product(listener_r3_orientation.at, listener_r3_orientation.up);
		const auto cosine = dot_product(voice_direction, listener_right);
		left_gain = 0.5 * (1.0 - cosine);
		right_gain = 0.5 * (1.0 + cosine);
	}
	else
	{
		left_gain = 0.5;
		right_gain = 0.5;
	}
}

} // bstone
