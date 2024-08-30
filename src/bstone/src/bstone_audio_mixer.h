/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// Notes:
//   - Coordinate system:
//     X points right, Y points up, Z - points towards the listener.
//
//   - `R2` or `r2` means `2D`.
//   - `R3` or `r3` means `3D`.
//

#ifndef BSTONE_AUDIO_MIXER_INCLUDED
#define BSTONE_AUDIO_MIXER_INCLUDED

#include <memory>
#include <utility>
#include "audio.h"
#include "bstone_audio_mixer_output_gains.h"
#include "bstone_audio_mixer_voice_handle.h"
#include "bstone_opl3.h"

namespace bstone
{

constexpr auto audio_mixer_min_gain = 0.0;
constexpr auto audio_mixer_max_gain = 1.0;
constexpr auto audio_mixer_default_gain = audio_mixer_max_gain;

enum class SoundType
{
	none,
	adlib_music,
	adlib_sfx,
	pc_speaker_sfx,
	pcm,
}; // SoundType

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
	AudioMixer() noexcept;
	virtual ~AudioMixer();

	virtual Opl3Type get_opl3_type() const = 0;
	virtual int get_rate() const = 0;
	virtual int get_channel_count() const = 0;
	virtual int get_mix_size_ms() const = 0;

	virtual void suspend_state() = 0;
	virtual void resume_state() = 0;

	virtual void set_mute(bool is_mute) = 0;
	virtual void set_gain(double gain) = 0;

	virtual void set_listener_r3_position(const AudioMixerListenerR3Position& r3_position) = 0;
	virtual void set_listener_r3_orientation(const AudioMixerListenerR3Orientation& r3_orientation) = 0;

	virtual AudioMixerVoiceHandle play_sound(const AudioMixerPlaySoundParam& param) = 0;

	virtual bool is_voice_playing(AudioMixerVoiceHandle voice_handle) const = 0;

	virtual void pause_voice(AudioMixerVoiceHandle voice_handle) = 0;
	virtual void resume_voice(AudioMixerVoiceHandle voice_handle) = 0;
	virtual void stop_voice(AudioMixerVoiceHandle voice_handle) = 0;

	virtual void set_voice_gain(AudioMixerVoiceHandle voice_handle, double gain) = 0;
	virtual void set_voice_r3_position(AudioMixerVoiceHandle voice_handle, const AudioMixerVoiceR3Position& r3_position) = 0;

	virtual bool can_set_voice_output_gains() const = 0;
	virtual void enable_set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		bool is_enable) = 0;
	virtual void set_voice_output_gains(
		AudioMixerVoiceHandle voice_handle,
		AudioMixerOutputGains& output_gains) = 0;
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

AudioMixerR3Vector operator*(const AudioMixerR3Vector& lhs, double rhs) noexcept;
AudioMixerVoiceR3Position operator*(const AudioMixerVoiceR3Position& lhs, double rhs) noexcept;
AudioMixerListenerR3Position operator*(const AudioMixerListenerR3Position& lhs, double rhs) noexcept;

// --------------------------------------------------------------------------

AudioMixerR3Vector operator-(const AudioMixerVoiceR3Position& lhs, const AudioMixerListenerR3Position& rhs) noexcept;

} // bstone


#endif // !BSTONE_AUDIO_MIXER_INCLUDED
