/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager (NULL)

#include "bstone_sys_audio_mgr_null.h"
#include "bstone_exception.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

namespace {

class AudioMgrNull final : public AudioMgr
{
public:
	AudioMgrNull(Logger& logger) noexcept;
	~AudioMgrNull() override;

private:
	Logger& logger_;

	bool do_is_initialized() const override;
	PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) override;

	[[noreturn]] static void not_initialized();
};

// --------------------------------------

AudioMgrNull::AudioMgrNull(Logger& logger) noexcept
	:
	logger_{logger}
{
	logger_.log_information("Start audio manager stub.");
}

AudioMgrNull::~AudioMgrNull()
{
	logger_.log_information("Shut down audio manager stub.");
}

bool AudioMgrNull::do_is_initialized() const
{
	return false;
}

PollingAudioDeviceUPtr AudioMgrNull::do_make_polling_audio_device(const PollingAudioDeviceOpenParam&)
{
	not_initialized();
}

[[noreturn]] void AudioMgrNull::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ======================================

AudioMgrUPtr make_audio_mgr_null(Logger& logger)
{
	return std::make_unique<AudioMgrNull>(logger);
}

} // namespace bstone::sys
