/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_EVENT_MGR_INCLUDED)
#define BSTONE_SYS_EVENT_MGR_INCLUDED

#include <memory>
#include "bstone_sys_event.h"

namespace bstone {
namespace sys {

class EventMgr
{
public:
	EventMgr() = default;
	virtual ~EventMgr() = default;

	bool poll_event(Event& e);

private:
	virtual bool do_poll_event(Event& e) = 0;
};

// ==========================================================================

using EventMgrUPtr = std::unique_ptr<EventMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_EVENT_MGR_INCLUDED
