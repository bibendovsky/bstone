/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// System manager

#ifndef BSTONE_SYS_SYSTEM_MGR_INCLUDED
#define BSTONE_SYS_SYSTEM_MGR_INCLUDED

#include "bstone_sys_audio_mgr.h"
#include "bstone_sys_event_mgr.h"
#include "bstone_sys_logger.h"
#include "bstone_sys_video_mgr.h"
#include <memory>

namespace bstone::sys {

class SystemMgr
{
public:
	SystemMgr() = default;
	virtual ~SystemMgr() = default;

	Logger& get_logger();
	AudioMgr& get_audio_mgr();
	EventMgr& get_event_mgr();
	VideoMgr& get_video_mgr();

private:
	virtual Logger& do_get_logger() = 0;
	virtual AudioMgr& do_get_audio_mgr() = 0;
	virtual EventMgr& do_get_event_mgr() = 0;
	virtual VideoMgr& do_get_video_mgr() = 0;
};

// ======================================

using SystemMgrUPtr = std::unique_ptr<SystemMgr>;

SystemMgrUPtr make_system_mgr(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_SYSTEM_MGR_INCLUDED
