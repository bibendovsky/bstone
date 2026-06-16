/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_MIXER_UTILS_INCLUDED
#define BSTONE_AUDIO_MIXER_UTILS_INCLUDED

#include "id_ca.h"
#include "bstone_audio_mixer.h"
#include <string>
#include <string_view>

namespace bstone {

class AudioMixerUtils
{
public:
	static AudioMixerR3Vector make_r3_position_from_w3d_coords(double w3d_x, double w3d_y, double w3d_z);
	static AudioMixerListenerR3Orientation make_listener_r3_orientation_from_w3d_view(double w3d_view_cos, double w3d_view_sin);
	// Returns the reciprocal module.
	static double get_r_module(const AudioMixerR3Vector& v);
	static AudioMixerR3Vector normalize(const AudioMixerR3Vector& v);
	static double get_distance(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b);
	static double dot_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b);
	static AudioMixerR3Vector cross_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b);

	// Notes:
	//    - All positions are expected in meters.
	//    - Listener's orientation should be normalized.
	static void spatialize_voice_2_0(
		const AudioMixerListenerR3Position& listener_r3_position,
		const AudioMixerListenerR3Orientation& listener_r3_orientation,
		const AudioMixerVoiceR3Position& voice_r3_position,
		double& left_gain,
		double& right_gain);
	static std::string_view get_sfx_chunk_name(int chunk_number, const AssetsInfo& assets_info);
	static std::string_view get_music_chunk_name(int chunk_number, const AssetsInfo& assets_info);
	static void append_sfx_chunk_dirname(const AssetsInfo& assets_info, std::string& pathname);
	static void append_music_chunk_dirname(const AssetsInfo& assets_info, std::string& pathname);
	static void append_sfx_chunk_pathname(int chunk_number, const AssetsInfo& assets_info, std::string& pathname);
	static void append_music_chunk_pathname(int chunk_number, const AssetsInfo& assets_info, std::string& pathname);

private:
	class Impl;
};

} // namespace bstone

#endif // BSTONE_AUDIO_MIXER_UTILS_INCLUDED
