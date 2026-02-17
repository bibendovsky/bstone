/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio device feeding via callback

#ifndef BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED
#define BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED

#include <memory>

namespace bstone::sys {

class PollingAudioDeviceCallback
{
public:
	PollingAudioDeviceCallback() = default;
	virtual ~PollingAudioDeviceCallback() = default;

	virtual void invoke(float* samples, int sample_count) = 0;
};

// ======================================

class PollingAudioDevice
{
public:
	PollingAudioDevice() = default;
	virtual ~PollingAudioDevice() = default;

	virtual int get_rate() const = 0;
	virtual int get_channel_count() const = 0;
	virtual int get_frame_count() const = 0;
	virtual void pause(bool is_pause) = 0;
};

// ======================================

struct PollingAudioDeviceOpenParam
{
	int desired_rate;
	int channel_count;
	int desired_frame_count;
	PollingAudioDeviceCallback* callback;
};

using PollingAudioDeviceUPtr = std::unique_ptr<PollingAudioDevice>;

} // namespace bstone::sys

#endif // BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED
