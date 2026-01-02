/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio device feeding via callback

#include "bstone_sys_polling_audio_device.h"

namespace bstone::sys {

void PollingAudioDeviceCallback::invoke(float* samples, int sample_count)
{
	do_invoke(samples, sample_count);
}

// ======================================

int PollingAudioDevice::get_rate() const
{
	return do_get_rate();
}

int PollingAudioDevice::get_channel_count() const
{
	return do_get_channel_count();
}

int PollingAudioDevice::get_frame_count() const
{
	return do_get_frame_count();
}

void PollingAudioDevice::pause(bool is_pause)
{
	do_pause(is_pause);
}

} // namespace bstone::sys
