/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_audio_mixer_utils.h"
#include "3d_def.h"
#include "bstone_assert.h"
#include "bstone_fs_utils.h"
#include "bstone_math.h"
#include <cassert>
#include <cmath>
#include <algorithm>
#include <type_traits>

namespace bstone {

AudioMixerR3Vector AudioMixerUtils::make_r3_position_from_w3d_coords(double w3d_x, double w3d_y, double w3d_z)
{
	return AudioMixerR3Vector{w3d_x, w3d_z, w3d_y};
}

AudioMixerListenerR3Orientation AudioMixerUtils::make_listener_r3_orientation_from_w3d_view(double w3d_view_cos, double w3d_view_sin)
{
	return AudioMixerListenerR3Orientation{
		.at = AudioMixerR3Vector{w3d_view_cos, 0.0, -w3d_view_sin},
		.up = AudioMixerR3Vector{0.0, 1.0, 0.0}};
}

double AudioMixerUtils::get_r_module(const AudioMixerR3Vector& v)
{
	return 1.0 / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

AudioMixerR3Vector AudioMixerUtils::normalize(const AudioMixerR3Vector& v)
{
	return v * get_r_module(v);
}

double AudioMixerUtils::get_distance(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b)
{
	const double dx = a.x - b.x;
	const double dy = a.y - b.y;
	const double dz = a.z - b.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}

double AudioMixerUtils::dot_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

AudioMixerR3Vector AudioMixerUtils::cross_product(const AudioMixerR3Vector& a, const AudioMixerR3Vector& b)
{
	return AudioMixerR3Vector{a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// Notes:
//    - All positions are expected in meters.
//    - Listener's orientation should be normalized.
void AudioMixerUtils::spatialize_voice_2_0(
	const AudioMixerListenerR3Position& listener_r3_position,
	const AudioMixerListenerR3Orientation& listener_r3_orientation,
	const AudioMixerVoiceR3Position& voice_r3_position,
	double& left_gain,
	double& right_gain)
{
	constexpr double distance_epsilon = 0.000'5;
	const auto distance = get_distance(listener_r3_position, voice_r3_position);

	if (distance > distance_epsilon)
	{
		const AudioMixerR3Vector voice_direction = normalize(voice_r3_position - listener_r3_position);
		const AudioMixerR3Vector listener_right = cross_product(listener_r3_orientation.at, listener_r3_orientation.up);
		const double cosine = dot_product(voice_direction, listener_right);
		left_gain = 0.5 * (1.0 - cosine);
		right_gain = 0.5 * (1.0 + cosine);
	}
	else
	{
		left_gain = 0.5;
		right_gain = 0.5;
	}
}

// =====================================

class AudioMixerUtils::Impl
{
public:
	inline constinit static const std::string_view unknown_name = "?";
	inline constinit static const std::string_view music_name = "music";
	inline constinit static const std::string_view sounds_name = "sounds";

	static void append_chunk_dirname(
		std::string_view sound_category_name,
		const AssetsInfo& assets_info,
		std::string& pathname);
};

void AudioMixerUtils::Impl::append_chunk_dirname(
	std::string_view sound_category_name,
	const AssetsInfo& assets_info,
	std::string& pathname)
{
	fs_utils::append_path_inplace(pathname, assets_info.get_base_path_name());
	if (sound_category_name.empty())
		fs_utils::append_path_inplace(pathname, unknown_name);
	else
		fs_utils::append_path_inplace(pathname, sound_category_name);
}

std::string_view AudioMixerUtils::get_sfx_chunk_name(int chunk_number, const AssetsInfo& assets_info)
{
	constexpr const char* shared_name_0 = "hitwall";
	constexpr const char* shared_name_1 = "term_type";
	constexpr const char* shared_name_2 = "getpistol";
	constexpr const char* shared_name_3 = "liquiddie";
	constexpr const char* shared_name_4 = "movegun2";
	constexpr const char* shared_name_5 = "movegun1";
	constexpr const char* shared_name_6 = "noway";
	constexpr const char* shared_name_7 = "scout_alert";
	constexpr const char* shared_name_8 = "gurney";
	constexpr const char* shared_name_9 = "playerdeath";
	constexpr const char* shared_name_10 = "concessions";
	constexpr const char* shared_name_11 = "atkioncannon";
	constexpr const char* shared_name_12 = "getkey";
	constexpr const char* shared_name_13 = "warpout";
	constexpr const char* shared_name_14 = "warpin";
	constexpr const char* shared_name_15 = "robot_servo";
	constexpr const char* shared_name_16 = "informantdeath";
	constexpr const char* shared_name_17 = "goldsternhalt";
	constexpr const char* shared_name_18 = "opendoor";
	constexpr const char* shared_name_19 = "closedoor";
	constexpr const char* shared_name_21 = "halt";
	constexpr const char* shared_name_22 = "rentdeath2";
	constexpr const char* shared_name_23 = "atkautocharge";
	constexpr const char* shared_name_24 = "atkcharged";
	constexpr const char* shared_name_26 = "atkburstrifle";
	constexpr const char* shared_name_27 = "vital_gone";
	constexpr const char* shared_name_28 = "shootdoor";
	constexpr const char* shared_name_29 = "rentdeath1";
	constexpr const char* shared_name_30 = "getburstrifle";
	constexpr const char* shared_name_31 = "getammo";
	constexpr const char* shared_name_32 = "shoot";
	constexpr const char* shared_name_33 = "health1";
	constexpr const char* shared_name_34 = "health2";
	constexpr const char* shared_name_35 = "bonus1";
	constexpr const char* shared_name_36 = "bonus2";
	constexpr const char* shared_name_37 = "bonus3";
	constexpr const char* shared_name_38 = "getioncannon";
	constexpr const char* shared_name_39 = "escpressed";
	constexpr const char* shared_name_40 = "elecappear";
	constexpr const char* shared_name_41 = "extra_man";
	constexpr const char* shared_name_42 = "elev_button";
	constexpr const char* shared_name_43 = "interrogate";
	constexpr const char* shared_name_44 = "bonus5";
	constexpr const char* shared_name_45 = "bonus4";
	constexpr const char* shared_name_46 = "pushwall";
	constexpr const char* shared_name_47 = "term_beep";
	constexpr const char* shared_name_48 = "roll_score";
	constexpr const char* shared_name_49 = "turret";
	constexpr const char* shared_name_50 = "explode1";
	constexpr const char* shared_name_52 = "swatdie";
	constexpr const char* shared_name_53 = "gguardhalt";
	constexpr const char* shared_name_54 = "explode2";
	constexpr const char* shared_name_55 = "blueboyhalt";
	constexpr const char* shared_name_56 = "proguarddeath";
	constexpr const char* shared_name_57 = "dogboyhalt";
	constexpr const char* shared_name_58 = "engine_thrust";
	constexpr const char* shared_name_59 = "scanhalt";
	constexpr const char* shared_name_60 = "getcannon";
	constexpr const char* shared_name_61 = "lcanhalt";
	constexpr const char* shared_name_62 = "prohalt";
	constexpr const char* shared_name_63 = "gguarddeath";
	constexpr const char* shared_name_64 = "blueboydeath";
	constexpr const char* shared_name_65 = "goldsternlaugh";
	constexpr const char* shared_name_66 = "scientisthalt";
	constexpr const char* shared_name_67 = "scientistdeath";
	constexpr const char* shared_name_68 = "dogboydeath";
	constexpr const char* shared_name_69 = "h_beat";
	constexpr const char* shared_name_70 = "swathalt";
	constexpr const char* shared_name_71 = "scandeath";
	constexpr const char* shared_name_72 = "lcandeath";
	constexpr const char* shared_name_73 = "informdeath2";
	constexpr const char* shared_name_74 = "informdeath3";
	constexpr const char* shared_name_75 = "gurneydeath";
	constexpr const char* shared_name_76 = "prodeath2";
	constexpr const char* shared_name_78 = "swatdeath2";
	constexpr const char* shared_name_79 = "lcanbreak";
	constexpr const char* shared_name_80 = "scanbreak";
	constexpr const char* shared_name_81 = "htechdooropen";
	constexpr const char* shared_name_82 = "htechdoorclose";
	constexpr const char* shared_name_83 = "elecarcdamage";
	constexpr const char* shared_name_84 = "podhatch";
	constexpr const char* shared_name_85 = "electshot";
	constexpr const char* shared_name_86 = "elecdie";
	constexpr const char* shared_name_87 = "atkgrenade";
	constexpr const char* shared_name_88 = "clawattack";
	constexpr const char* shared_name_89 = "punchattack";
	constexpr const char* shared_name_90 = "spitattack";
	constexpr const char* shared_name_91 = "poddeath";
	constexpr const char* shared_name_92 = "podhalt";
	constexpr const char* shared_name_94 = "scideath2";
	constexpr const char* shared_name_95 = "scideath3";
	constexpr const char* shared_name_96 = "gottoken";
	constexpr const char* shared_name_97 = "switch";
	constexpr const char* shared_name_98 = "stats1";
	constexpr const char* shared_name_99 = "stats2";

	constexpr const char* aog_name_20 = "__unused__";
	constexpr const char* aog_name_25 = "rentdeath3";
	constexpr const char* aog_name_51 = "__unused_2__";
	constexpr const char* aog_name_77 = "prodeath3";
	constexpr const char* aog_name_93 = "swatdeath3";

	constexpr const char* ps_name_20 = "getdetonator";
	constexpr const char* ps_name_25 = "radar_powerup";
	constexpr const char* ps_name_51 = "__unused_51__";
	constexpr const char* ps_name_77 = "__unused_77__";
	constexpr const char* ps_name_93 = "__unused_93__";

	constinit static const std::string_view aog_names[] = {
		shared_name_0,
		shared_name_1,
		shared_name_2,
		shared_name_3,
		shared_name_4,
		shared_name_5,
		shared_name_6,
		shared_name_7,
		shared_name_8,
		shared_name_9,
		shared_name_10,
		shared_name_11,
		shared_name_12,
		shared_name_13,
		shared_name_14,
		shared_name_15,
		shared_name_16,
		shared_name_17,
		shared_name_18,
		shared_name_19,
		aog_name_20,
		shared_name_21,
		shared_name_22,
		shared_name_23,
		shared_name_24,
		aog_name_25,
		shared_name_26,
		shared_name_27,
		shared_name_28,
		shared_name_29,
		shared_name_30,
		shared_name_31,
		shared_name_32,
		shared_name_33,
		shared_name_34,
		shared_name_35,
		shared_name_36,
		shared_name_37,
		shared_name_38,
		shared_name_39,
		shared_name_40,
		shared_name_41,
		shared_name_42,
		shared_name_43,
		shared_name_44,
		shared_name_45,
		shared_name_46,
		shared_name_47,
		shared_name_48,
		shared_name_49,
		shared_name_50,
		aog_name_51,
		shared_name_52,
		shared_name_53,
		shared_name_54,
		shared_name_55,
		shared_name_56,
		shared_name_57,
		shared_name_58,
		shared_name_59,
		shared_name_60,
		shared_name_61,
		shared_name_62,
		shared_name_63,
		shared_name_64,
		shared_name_65,
		shared_name_66,
		shared_name_67,
		shared_name_68,
		shared_name_69,
		shared_name_70,
		shared_name_71,
		shared_name_72,
		shared_name_73,
		shared_name_74,
		shared_name_75,
		shared_name_76,
		aog_name_77,
		shared_name_78,
		shared_name_79,
		shared_name_80,
		shared_name_81,
		shared_name_82,
		shared_name_83,
		shared_name_84,
		shared_name_85,
		shared_name_86,
		shared_name_87,
		shared_name_88,
		shared_name_89,
		shared_name_90,
		shared_name_91,
		shared_name_92,
		aog_name_93,
		shared_name_94,
		shared_name_95,
		shared_name_96,
		shared_name_97,
		shared_name_98,
		shared_name_99,
	};
	constinit static const std::string_view ps_names[] = {
		shared_name_0,
		shared_name_1,
		shared_name_2,
		shared_name_3,
		shared_name_4,
		shared_name_5,
		shared_name_6,
		shared_name_7,
		shared_name_8,
		shared_name_9,
		shared_name_10,
		shared_name_11,
		shared_name_12,
		shared_name_13,
		shared_name_14,
		shared_name_15,
		shared_name_16,
		shared_name_17,
		shared_name_18,
		shared_name_19,
		ps_name_20,
		shared_name_21,
		shared_name_22,
		shared_name_23,
		shared_name_24,
		ps_name_25,
		shared_name_26,
		shared_name_27,
		shared_name_28,
		shared_name_29,
		shared_name_30,
		shared_name_31,
		shared_name_32,
		shared_name_33,
		shared_name_34,
		shared_name_35,
		shared_name_36,
		shared_name_37,
		shared_name_38,
		shared_name_39,
		shared_name_40,
		shared_name_41,
		shared_name_42,
		shared_name_43,
		shared_name_44,
		shared_name_45,
		shared_name_46,
		shared_name_47,
		shared_name_48,
		shared_name_49,
		shared_name_50,
		ps_name_51,
		shared_name_52,
		shared_name_53,
		shared_name_54,
		shared_name_55,
		shared_name_56,
		shared_name_57,
		shared_name_58,
		shared_name_59,
		shared_name_60,
		shared_name_61,
		shared_name_62,
		shared_name_63,
		shared_name_64,
		shared_name_65,
		shared_name_66,
		shared_name_67,
		shared_name_68,
		shared_name_69,
		shared_name_70,
		shared_name_71,
		shared_name_72,
		shared_name_73,
		shared_name_74,
		shared_name_75,
		shared_name_76,
		ps_name_77,
		shared_name_78,
		shared_name_79,
		shared_name_80,
		shared_name_81,
		shared_name_82,
		shared_name_83,
		shared_name_84,
		shared_name_85,
		shared_name_86,
		shared_name_87,
		shared_name_88,
		shared_name_89,
		shared_name_90,
		shared_name_91,
		shared_name_92,
		ps_name_93,
		shared_name_94,
		shared_name_95,
		shared_name_96,
		shared_name_97,
		shared_name_98,
		shared_name_99,
	};
	if (chunk_number < 0 || chunk_number >= NUMSOUNDS)
	{
		BSTONE_ASSERT(false && "Chunk number out of range.");
		return Impl::unknown_name;
	}
	if (assets_info.is_aog())
		return aog_names[chunk_number];
	else if (assets_info.is_ps())
		return ps_names[chunk_number];
	else
	{
		BSTONE_ASSERT(false && "Unknown product.");
		return Impl::unknown_name;
	}
}

std::string_view AudioMixerUtils::get_music_chunk_name(int chunk_number, const AssetsInfo& assets_info)
{
	constinit static const std::string_view aog_names[] = {
		"s2100a",   // 0
		"golda",    // 1
		"apogfnfm", // 2
		"drkhalla", // 3
		"freedoma", // 4
		"genefunk", // 5
		"timea",    // 6
		"hidinga",  // 7
		"incnratn", // 8
		"junglea",  // 9
		"levela",   // 10
		"meetinga", // 11
		"struta",   // 12
		"racshufl", // 13
		"rumbaa",   // 14
		"searchna", // 15
		"theme",    // 16
		"thewaya",  // 17
		"intrigea", // 18
	};
	constexpr int aog_count = std::extent_v<decltype(aog_names)>;
	constinit static const std::string_view ps_names[] = {
		"catacomb", // 0
		"sticks",   // 1
		"apogfnfm", // 2
		"plot",     // 3
		"circles",  // 4
		"lastlaff", // 5
		"tohell",   // 6
		"fortress", // 7
		"giving",   // 8
		"hartbeat", // 9
		"lurking",  // 10
		"majmin",   // 11
		"vaccinap", // 12
		"darkness", // 13
		"monastry", // 14
		"tombp",    // 15
		"time",     // 16
		"mourning", // 17
		"serpent",  // 18
		"hiscore",  // 19
		"theme",    // 20
	};
	constexpr int ps_count = std::extent_v<decltype(ps_names)>;
	if (assets_info.is_aog())
	{
		if (chunk_number < 0 || chunk_number >= aog_count)
		{
			BSTONE_ASSERT(false && "Chunk number out of range.");
			return Impl::unknown_name;
		}
		return aog_names[chunk_number];
	}
	else if (assets_info.is_ps())
	{
		if (chunk_number < 0 || chunk_number >= ps_count)
		{
			BSTONE_ASSERT(false && "Chunk number out of range.");
			return Impl::unknown_name;
		}
		return ps_names[chunk_number];
	}
	else
	{
		BSTONE_ASSERT(false && "Unknown product.");
		return Impl::unknown_name;
	}
}

void AudioMixerUtils::append_sfx_chunk_dirname(const AssetsInfo& assets_info, std::string& pathname)
{
	Impl::append_chunk_dirname(Impl::sounds_name, assets_info, pathname);
}

void AudioMixerUtils::append_music_chunk_dirname(const AssetsInfo& assets_info, std::string& pathname)
{
	Impl::append_chunk_dirname(Impl::music_name, assets_info, pathname);
}

void AudioMixerUtils::append_sfx_chunk_pathname(int chunk_number, const AssetsInfo& assets_info, std::string& pathname)
{
	append_sfx_chunk_dirname(assets_info, pathname);
	fs_utils::append_path_inplace(pathname, get_sfx_chunk_name(chunk_number, assets_info));
}

void AudioMixerUtils::append_music_chunk_pathname(int chunk_number, const AssetsInfo& assets_info, std::string& pathname)
{
	append_music_chunk_dirname(assets_info, pathname);
	fs_utils::append_path_inplace(pathname, get_music_chunk_name(chunk_number, assets_info));
}

} // namespace bstone
