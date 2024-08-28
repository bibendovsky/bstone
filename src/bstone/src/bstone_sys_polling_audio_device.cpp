/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio device feedding via callback.

#include "bstone_sys_polling_audio_device.h"

namespace bstone {
namespace sys {

PollingAudioDeviceCallback::PollingAudioDeviceCallback() = default;

PollingAudioDeviceCallback::~PollingAudioDeviceCallback() = default;

void PollingAudioDeviceCallback::invoke(float* samples, int sample_count)
{
	do_invoke(samples, sample_count);
}

// ==========================================================================

PollingAudioDevice::PollingAudioDevice() = default;

PollingAudioDevice::~PollingAudioDevice() = default;

int PollingAudioDevice::get_rate() const noexcept
{
	return do_get_rate();
}

int PollingAudioDevice::get_channel_count() const noexcept
{
	return do_get_channel_count();
}

int PollingAudioDevice::get_frame_count() const noexcept
{
	return do_get_frame_count();
}

void PollingAudioDevice::pause(bool is_pause)
{
	do_pause(is_pause);
}

} // namespace sys
} // namespace bstone
