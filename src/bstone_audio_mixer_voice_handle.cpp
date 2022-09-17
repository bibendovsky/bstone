/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_audio_mixer_voice_handle.h"
#include <limits>
#include "bstone_exception.h"

namespace bstone
{

namespace
{

class AudioMixerVoiceHandleException : public Exception
{
public:
	explicit AudioMixerVoiceHandleException(const char* message) noexcept
		:
		Exception{"AUDIO_MIXER_VOICE_HANDLE", message}
	{
	}
}; // AudioMixerVoiceHandleException

} // namespace

// ==========================================================================

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
try
{
	static constexpr auto max_value = std::numeric_limits<AudioMixerVoiceHandleValue>::max();

	if (get() == max_value)
	{
		fail("Value overflow.");
	}

	++value_;

	return *this;
}
catch (...)
{
	fail_nested(__func__);
}

[[noreturn]] void AudioMixerVoiceHandle::fail(const char* message)
{
	throw AudioMixerVoiceHandleException{message};
}

[[noreturn]] void AudioMixerVoiceHandle::fail_nested(const char* message)
{
	std::throw_with_nested(AudioMixerVoiceHandleException{message});
}

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
