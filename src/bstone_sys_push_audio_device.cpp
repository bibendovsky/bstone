/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_push_audio_device.h"

namespace bstone {
namespace sys {

void PushAudioDeviceCallback::invoke(float* samples, int sample_count)
{
	do_invoke(samples, sample_count);
}

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
{
	do_pause(is_pause);
}

} // namespace sys
} // namespace bstone
