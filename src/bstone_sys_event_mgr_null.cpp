/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_event_mgr_null.h"

#include <cassert>

#include "bstone_exception.h"
#include "bstone_single_pool_resource.h"
#include "bstone_sys_logger.h"

#define BSTONE_SDL_2_0_4 SDL_VERSION_ATLEAST(2, 0, 4)

namespace bstone {
namespace sys {

namespace {

class NullEventMgr final : public EventMgr
{
public:
	NullEventMgr(Logger& logger);
	~NullEventMgr() override;

	void* operator new(std::size_t size);
	void operator delete(void* ptr) noexcept;

private:
	Logger& logger_;

private:
	bool do_is_initialized() const noexcept override;

	bool do_poll_event(Event& e) override;

private:
	[[noreturn]] static void not_initialized();
};

// ==========================================================================

using NullEventMgrPool = SinglePoolResource<NullEventMgr>;
NullEventMgrPool null_event_mgr_pool{};

// ==========================================================================

NullEventMgr::NullEventMgr(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up NULL event manager.");
}

NullEventMgr::~NullEventMgr()
{
	logger_.log_information("Shut down NULL event manager.");
}

void* NullEventMgr::operator new(std::size_t size)
try {
	return null_event_mgr_pool.allocate(size);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

void NullEventMgr::operator delete(void* ptr) noexcept
{
	null_event_mgr_pool.deallocate(ptr);
}

bool NullEventMgr::do_is_initialized() const noexcept
{
	return false;
}

bool NullEventMgr::do_poll_event(Event&)
{
	not_initialized();
}

[[noreturn]] void NullEventMgr::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ==========================================================================

EventMgrUPtr make_null_event_mgr(Logger& logger)
try {
	return std::make_unique<NullEventMgr>(logger);
} BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
