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

inline constexpr AudioMixerVoiceHandleValue audio_mixer_voice_invalid_handle_value{};

// =====================================

class AudioMixerVoiceHandle
{
public:
	AudioMixerVoiceHandle() = default;

	AudioMixerVoiceHandleValue get() const;
	bool is_valid() const;
	void reset();
	AudioMixerVoiceHandle& operator++();

private:
	AudioMixerVoiceHandleValue value_{audio_mixer_voice_invalid_handle_value};
};

// =====================================

struct AudioMixerVoiceHandleStdHasher
{
	std::size_t operator()(AudioMixerVoiceHandle key) const;
};

// =====================================

bool operator==(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs);
bool operator!=(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs);

} // namespace bstone

#endif // BSTONE_AUDIO_MIXER_VOICE_HANDLE_INCLUDED
