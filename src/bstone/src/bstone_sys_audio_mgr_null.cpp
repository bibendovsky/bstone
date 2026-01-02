/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager stub.

#include "bstone_sys_audio_mgr_null.h"
#include "bstone_exception.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

namespace {

class NullAudioMgr final : public AudioMgr
{
public:
	NullAudioMgr(Logger& logger) noexcept;
	~NullAudioMgr() override;

private:
	Logger& logger_;

	bool do_is_initialized() const override;
	PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) override;

	[[noreturn]] static void not_initialized();
};

// --------------------------------------

NullAudioMgr::NullAudioMgr(Logger& logger) noexcept
	:
	logger_{logger}
{
	logger_.log_information("Start audio manager stub.");
}

NullAudioMgr::~NullAudioMgr()
{
	logger_.log_information("Shut down audio manager stub.");
}

bool NullAudioMgr::do_is_initialized() const
{
	return false;
}

PollingAudioDeviceUPtr AudioMgrNull::do_make_polling_audio_device([[maybe_unused]] const PollingAudioDeviceOpenParam& param)
{
	not_initialized();
}

[[noreturn]] void NullAudioMgr::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ======================================

AudioMgrUPtr make_null_audio_mgr(Logger& logger)
{
	return std::make_unique<NullAudioMgr>(logger);
}

} // namespace bstone::sys
