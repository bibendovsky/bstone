/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
	AudioMixerVoiceHandle() noexcept;

	AudioMixerVoiceHandleValue get() const noexcept;
	bool is_valid() const noexcept;
	void reset() noexcept;
	AudioMixerVoiceHandle& operator++();

private:
	AudioMixerVoiceHandleValue value_{audio_mixer_voice_invalid_handle_value};
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
