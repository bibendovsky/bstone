/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_push_audio_device.h"

namespace bstone {
namespace sys {

void PushAudioDeviceCallback::invoke(float* samples, int sample_count)
BSTONE_BEGIN_FUNC_TRY
	do_invoke(samples, sample_count);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

// ==========================================================================

int PushAudioDevice::get_rate() const noexcept
{
	return do_get_rate();
}

int PushAudioDevice::get_channel_count() const noexcept
{
	return do_get_channel_count();
}

int PushAudioDevice::get_frame_count() const noexcept
{
	return do_get_frame_count();
}

void PushAudioDevice::pause(bool is_pause)
BSTONE_BEGIN_FUNC_TRY
	do_pause(is_pause);
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
