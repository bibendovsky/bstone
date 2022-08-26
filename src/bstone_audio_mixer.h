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

//
// Notes:
//   - Coordinate system:
//     X points right, Y points up, Z - points towards the listener.
//
//   - Inverse distance clamped model:
//     d = clamp(d, d_min, d_max)
//     gain = d_min / (d_min + rof * (d - d_min))
//
//   - Linear distance clamped model:
//     d = clamp(d, d_min, d_max)
//     gain = 1 - rof * (d - d_min) / (d_max - d_min);
//
//   - `R2` or `r2` means `2D`.
//   - `R3` or `r3` means `3D`.
//

#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED

#include <memory>
#include <utility>
#include "audio.h"
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_opl3.h"

namespace bstone
{

constexpr auto audio_mixer_default_meters_per_units = 1.0;

constexpr auto audio_mixer_min_gain = 0.0;
constexpr auto audio_mixer_max_gain = 1.0;
constexpr auto audio_mixer_default_gain = audio_mixer_max_gain;

constexpr auto audio_mixer_min_min_distance = 0.0;
constexpr auto audio_mixer_default_min_distance = 1.0;

constexpr auto audio_mixer_min_max_distance = 0.0;
constexpr auto audio_mixer_default_max_distance = 1'000'000'000.0;

constexpr auto audio_mixer_min_rolloff_factor = 0.0;
constexpr auto audio_mixer_default_rolloff_factor = 1.0;

enum class SoundType
{
	none,
	adlib_music,
	adlib_sfx,
	pc_speaker_sfx,
	pcm,
}; // SoundType

enum class AudioMixerDistanceModel
{
	none,
	inverse_clamped,
	linear_clamped,
}; // AudioMixerDistanceModel

constexpr auto audio_mixer_default_distance_model = AudioMixerDistanceModel::inverse_clamped;

struct AudioMixerR3Vector
{
	double x;
	double y;
	double z;
}; // AudioMixerR3Vector

struct AudioMixerListenerR3Position : AudioMixerR3Vector
{
	template<typename ...UArgs>
	explicit AudioMixerListenerR3Position(UArgs&& ...args) noexcept
		:
		AudioMixerR3Vector{std::forward<UArgs>(args)...}
	{
	}

	void operator=(const AudioMixerR3Vector& r3_vector) noexcept;
}; // AudioMixerListenerR3Position

AudioMixerListenerR3Position audio_mixer_make_default_listener_r3_position() noexcept;

struct AudioMixerListenerR3Orientation
{
	AudioMixerR3Vector at;
	AudioMixerR3Vector up;
}; // AudioMixerListenerR3Orientation

AudioMixerR3Vector audio_mixer_make_default_listener_r3_orientation_at() noexcept;
AudioMixerR3Vector audio_mixer_make_default_listener_r3_orientation_up() noexcept;
AudioMixerListenerR3Orientation audio_mixer_make_default_listener_r3_orientation() noexcept;

struct AudioMixerVoiceR3Position : AudioMixerR3Vector
{
	AudioMixerVoiceR3Position(const AudioMixerR3Vector& r3_vector) noexcept
		:
		AudioMixerR3Vector{r3_vector}
	{
	}

	template<typename ...UArgs>
	explicit AudioMixerVoiceR3Position(UArgs&& ...args) noexcept
		:
		AudioMixerR3Vector{std::forward<UArgs>(args)...}
	{
	}
}; // AudioMixerVoiceR3Position

AudioMixerVoiceR3Position audio_mixer_make_default_voice_r3_position() noexcept;

struct AudioMixerVoiceR3Attenuation
{
	double min_distance;
	double max_distance;
	double rolloff_factor;
}; // AudioMixerVoiceR3Attenuation

AudioMixerVoiceR3Attenuation audio_mixer_make_default_voice_attenuation() noexcept;

struct AudioMixerInitParam
{
	AudioDriverType audio_driver_type;

	Opl3Type opl3_type;
	int dst_rate;

	int mix_size_ms; // (milliseconds)
	int max_voices;
}; // AudioMixerInitParam

struct AudioMixerPlaySoundParam
{
	SoundType sound_type;
	int sound_index;
	const void* data;
	int data_size;
	bool is_looping;
	bool is_r3;
}; // AudioMixerPlaySoundParam

// ==========================================================================

class AudioMixer
{
public:
	AudioMixer() noexcept = default;
	virtual ~AudioMixer() = default;

	virtual Opl3Type get_opl3_type() const = 0;
	virtual int get_rate() const = 0;
	virtual int get_channel_count() const = 0;
	virtual int get_mix_size_ms() const = 0;

	virtual void suspend_state() = 0;
	virtual void resume_state() = 0;

	virtual void set_mute(bool is_mute) = 0;
	virtual void set_distance_model(AudioMixerDistanceModel distance_model) = 0;

	virtual void set_listener_meters_per_unit(double meters_per_unit) = 0;
	virtual void set_listener_r3_position(const AudioMixerListenerR3Position& r3_position) = 0;
	virtual void set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation) = 0;

	virtual AudioMixerVoiceHandle play_sound(const AudioMixerPlaySoundParam& param) = 0;

	virtual bool is_voice_playing(AudioMixerVoiceHandle voice_handle) const = 0;

	virtual void pause_voice(AudioMixerVoiceHandle voice_handle) = 0;
	virtual void resume_voice(AudioMixerVoiceHandle voice_handle) = 0;
	virtual void stop_voice(AudioMixerVoiceHandle voice_handle) = 0;

	virtual void set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain) = 0;
	virtual void set_voice_r3_attenuation(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Attenuation& r3_attenuation) = 0;
	virtual void set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position) = 0;
}; // AudioMixer

// ==========================================================================

using AudioMixerUPtr = std::unique_ptr<AudioMixer>;

AudioMixerUPtr make_audio_mixer(const AudioMixerInitParam& param);

// ==========================================================================

bool operator==(const AudioMixerR3Vector& lhs, const AudioMixerR3Vector& rhs) noexcept;
bool operator!=(const AudioMixerR3Vector& lhs, const AudioMixerR3Vector& rhs) noexcept;

// --------------------------------------------------------------------------

bool operator==(const AudioMixerListenerR3Orientation& lhs, const AudioMixerListenerR3Orientation& rhs) noexcept;
bool operator!=(const AudioMixerListenerR3Orientation& lhs, const AudioMixerListenerR3Orientation& rhs) noexcept;

// --------------------------------------------------------------------------

bool operator==(const AudioMixerVoiceR3Attenuation& lhs, const AudioMixerVoiceR3Attenuation& rhs) noexcept;
bool operator!=(const AudioMixerVoiceR3Attenuation& lhs, const AudioMixerVoiceR3Attenuation& rhs) noexcept;

// --------------------------------------------------------------------------

AudioMixerR3Vector operator*(const AudioMixerR3Vector& lhs, double rhs) noexcept;
AudioMixerVoiceR3Position operator*(const AudioMixerVoiceR3Position& lhs, double rhs) noexcept;
AudioMixerListenerR3Position operator*(const AudioMixerListenerR3Position& lhs, double rhs) noexcept;

// --------------------------------------------------------------------------

AudioMixerR3Vector operator-(const AudioMixerVoiceR3Position& lhs, const AudioMixerListenerR3Position& rhs) noexcept;

} // bstone


#endif // !BSTONE_AUDIO_MIXER_INCLUDED
