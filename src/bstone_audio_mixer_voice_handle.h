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
