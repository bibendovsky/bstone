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

#include "bstone_audio_mixer_voice_handle_mgr.h"
#include "bstone_exception.h"

namespace bstone
{

namespace
{

class AudioMixerVoiceHandleMgrException : public Exception
{
public:
	explicit AudioMixerVoiceHandleMgrException(const char* message) noexcept
		:
		Exception{"AUDIO_MIXER_VOICE_HANDLE_MGR", message}
	{
	}
}; // AudioMixerVoiceHandleMgrException

} // namespace

// ==========================================================================

[[noreturn]]
void audio_mixer_voice_handle_mgr_fail(const char* message)
{
	throw AudioMixerVoiceHandleMgrException{message};
}

[[noreturn]]
void audio_mixer_voice_handle_mgr_fail_nested(const char* message)
{
	std::throw_with_nested(AudioMixerVoiceHandleMgrException{message});
}

} // bstone
