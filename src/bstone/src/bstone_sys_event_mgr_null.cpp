/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Event manager (NULL)

#include "bstone_sys_event_mgr_null.h"
#include "bstone_exception.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

namespace {

class EventMgrNull final : public EventMgr
{
public:
	EventMgrNull(Logger& logger);
	~EventMgrNull() override;

private:
	Logger& logger_;

	bool do_is_initialized() const override;
	bool do_poll_event(Event& e) override;

	[[noreturn]] static void not_initialized();
};

// --------------------------------------

EventMgrNull::EventMgrNull(Logger& logger)
	:
	logger_{logger}
{
	logger_.log_information("Start up NULL event manager.");
}

EventMgrNull::~EventMgrNull()
{
	logger_.log_information("Shut down NULL event manager.");
}

bool EventMgrNull::do_is_initialized() const
{
	return false;
}

bool EventMgrNull::do_poll_event([[maybe_unused]] Event& e)
{
	not_initialized();
}

[[noreturn]] void EventMgrNull::not_initialized()
{
	BSTONE_THROW_STATIC_SOURCE("Not initialized.");
}

} // namespace

// ======================================

EventMgrUPtr make_event_mgr_null(Logger& logger)
{
	return std::make_unique<EventMgrNull>(logger);
}

} // namespace bstone::sys
