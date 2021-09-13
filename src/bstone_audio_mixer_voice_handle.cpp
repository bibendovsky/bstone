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
