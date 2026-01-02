/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Event manager

#ifndef BSTONE_SYS_EVENT_MGR_INCLUDED
#define BSTONE_SYS_EVENT_MGR_INCLUDED

#include "bstone_sys_event.h"
#include <memory>

namespace bstone::sys {

class EventMgr
{
public:
	EventMgr() = default;
	virtual ~EventMgr() = default;

	bool is_initialized() const;
	bool poll_event(Event& e);

private:
	virtual bool do_is_initialized() const = 0;
	virtual bool do_poll_event(Event& e) = 0;
};

// ======================================

using EventMgrUPtr = std::unique_ptr<EventMgr>;

} // namespace bstone::sys

#endif // BSTONE_SYS_EVENT_MGR_INCLUDED
