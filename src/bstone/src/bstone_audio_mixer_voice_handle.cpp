/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_audio_mixer_voice_handle.h"
#include <limits>
#include "bstone_exception.h"

namespace bstone
{

AudioMixerVoiceHandle::AudioMixerVoiceHandle() noexcept = default;

AudioMixerVoiceHandleValue AudioMixerVoiceHandle::get() const noexcept
{
	return value_;
}

bool AudioMixerVoiceHandle::is_valid() const noexcept
{
	return get() != audio_mixer_voice_invalid_handle_value;
}

void AudioMixerVoiceHandle::reset() noexcept
{
	value_ = audio_mixer_voice_invalid_handle_value;
}

AudioMixerVoiceHandle& AudioMixerVoiceHandle::operator++()
try {
	static constexpr auto max_value = std::numeric_limits<AudioMixerVoiceHandleValue>::max();

	if (get() == max_value)
	{
		BSTONE_THROW_STATIC_SOURCE("Value overflow.");
	}

	++value_;

	return *this;
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

std::size_t AudioMixerVoiceHandleStdHasher::operator()(AudioMixerVoiceHandle key) const noexcept
{
	return static_cast<std::size_t>(key.get());
}

// ==========================================================================

bool operator==(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs) noexcept
{
	return lhs.get() == rhs.get();
}

bool operator!=(const AudioMixerVoiceHandle& lhs, const AudioMixerVoiceHandle& rhs) noexcept
{
	return !(lhs == rhs);
}

} // bstone
