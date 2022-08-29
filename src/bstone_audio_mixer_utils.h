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
