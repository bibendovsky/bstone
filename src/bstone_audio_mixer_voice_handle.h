/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_MIXER_VOICE_HANDLE_INCLUDED
#define BSTONE_AUDIO_MIXER_VOICE_HANDLE_INCLUDED

#include <cstddef>

namespace bstone
{

using AudioMixerVoiceHandleValue = std::size_t;

constexpr auto audio_mixer_voice_invalid_handle_value = AudioMixerVoiceHandleValue{};

// ==========================================================================

class AudioMixerVoiceHandle
{
public:
	AudioMixerVoiceHandle() noexcept = default;

	AudioMixerVoiceHandleValue get() const noexcept;
	bool is_valid() const noexcept;
	void reset() noexcept;
	AudioMixerVoiceHandle& operator++();

private:
	AudioMixerVoiceHandleValue value_{audio_mixer_voice_invalid_handle_value};

	[[noreturn]] static void fail(const char* message);
	[[noreturn]] static void fail_nested(const char* message);
}; // AudioMixerVoiceHandle

// ==========================================================================

struct AudioMixerVoiceHandleStdHasher
{
	std::size_t operator()(AudioMixerVoiceHandle key) const noexcept;
}; // AudioMixerVoiceHandleStdHasher

// ==========================================================================

bool operator==(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs) noexcept;
bool operator!=(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs) noexcept;

} // bstone

#endif // !BSTONE_AUDIO_MIXER_VOICE_HANDLE_INCLUDED
