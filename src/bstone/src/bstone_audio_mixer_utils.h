/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_MIXER_UTILS_INCLUDED
#define BSTONE_AUDIO_MIXER_UTILS_INCLUDED

#include "bstone_audio_mixer.h"

namespace bstone
{

struct AudioMixerUtils
{
	static AudioMixerR3Vector make_r3_position_from_w3d_coords(double w3d_x, double w3d_y, double w3d_z) noexcept;
	static AudioMixerListenerR3Orientation make_listener_r3_orientation_from_w3d_view(double w3d_view_cos, double w3d_view_sin) noexcept;
	// Returns the reciprocal module.
	static double get_r_module(const AudioMixerR3Vector& v) noexcept;
	static AudioMixerR3Vector normalize(const AudioMixerR3Vector& v) noexcept;
	static double get_distance(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept;
	static double dot_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept;
	static AudioMixerR3Vector cross_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b) noexcept;

	// Notes:
	//    - All positions are expected in meters.
	//    - Listener's orientation should be normalized.
	static void spatialize_voice_2_0(
		const AudioMixerListenerR3Position& listener_r3_position,
		const AudioMixerListenerR3Orientation& listener_r3_orientation,
		const AudioMixerVoiceR3Position& voice_r3_position,
		double& left_gain,
		double& right_gain) noexcept;
}; // AudioMixerUtils

} // bstone

#endif // !BSTONE_AUDIO_MIXER_UTILS_INCLUDED
