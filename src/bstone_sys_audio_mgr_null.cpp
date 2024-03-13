/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_audio_mgr_null.h"

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

namespace {

class NullAudioMgr final : public AudioMgr
{
public:
	NullAudioMgr(Logger& logger) noexcept;
	~NullAudioMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;

private:
	bool do_is_initialized() const noexcept override;
	PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) override;

private:
	[[noreturn]] static void not_initialized();
};

// ==========================================================================

using NullAudioMgrPool = SinglePoolResource<NullAudioMgr>;
NullAudioMgrPool null_audio_mgr_pool{};

// ==========================================================================

NullAudioMgr::NullAudioMgr(Logger& logger) noexcept
	:
	logger_{logger}
{
	logger_.log_information("Start up NULL audio manager.");
}

NullAudioMgr::~NullAudioMgr()
{
	logger_.log_information("Shut down NULL audio manager.");
}

void* NullAudioMgr::operator new(std::size_t size)
try {
	return null_audio_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullAudioMgr::operator delete(void* ptr) noexcept
{
	null_audio_mgr_pool.deallocate(ptr);
}

bool NullAudioMgr::do_is_initialized() const noexcept
{
	return false;
}

PollingAudioDeviceUPtr NullAudioMgr::do_make_polling_audio_device(const PollingAudioDeviceOpenParam&)
{
	not_initialized();
}

[[noreturn]] void NullAudioMgr::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ==========================================================================

AudioMgrUPtr make_null_audio_mgr(Logger& logger)
try {
	return std::make_unique<NullAudioMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
