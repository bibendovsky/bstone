/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
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
