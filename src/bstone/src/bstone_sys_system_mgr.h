/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// System manager.

#ifndef BSTONE_SYS_SYSTEM_MGR_INCLUDED
#define BSTONE_SYS_SYSTEM_MGR_INCLUDED

#include <memory>

#include "bstone_sys_audio_mgr.h"
#include "bstone_sys_event_mgr.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_video_mgr.h"

namespace bstone {
namespace sys {

class SystemMgr
{
public:
	SystemMgr();
	virtual ~SystemMgr();

	AudioMgr& get_audio_mgr();
	EventMgr& get_event_mgr();
	VideoMgr& get_video_mgr();

private:
	virtual AudioMgr& do_get_audio_mgr() = 0;
	virtual EventMgr& do_get_event_mgr() = 0;
	virtual VideoMgr& do_get_video_mgr() = 0;
};

// ==========================================================================

using SystemMgrUPtr = std::unique_ptr<SystemMgr>;

SystemMgrUPtr make_system_mgr(Logger& logger);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SYSTEM_MGR_INCLUDED
