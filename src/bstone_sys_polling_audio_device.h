/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio device feedding via callback.

#ifndef BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED
#define BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED

#include <memory>

namespace bstone {
namespace sys {

class PollingAudioDeviceCallback
{
public:
	PollingAudioDeviceCallback();
	virtual ~PollingAudioDeviceCallback();

	void invoke(float* samples, int sample_count);

private:
	virtual void do_invoke(float* samples, int sample_count) = 0;
};

// ==========================================================================

class PollingAudioDevice
{
public:
	PollingAudioDevice();
	virtual ~PollingAudioDevice();

	int get_rate() const noexcept;
	int get_channel_count() const noexcept;
	int get_frame_count() const noexcept;

	void pause(bool is_pause);

private:
	virtual int do_get_rate() const noexcept = 0;
	virtual int do_get_channel_count() const noexcept = 0;
	virtual int do_get_frame_count() const noexcept = 0;

	virtual void do_pause(bool is_pause) = 0;
};

// ==========================================================================

struct PollingAudioDeviceOpenParam
{
	int desired_rate;
	int channel_count;
	int desired_frame_count;
	PollingAudioDeviceCallback* callback;
};

using PollingAudioDeviceUPtr = std::unique_ptr<PollingAudioDevice>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_POLLING_AUDIO_DEVICE_INCLUDED
