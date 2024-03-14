/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_ccmd_mgr.h"
#include "bstone_cvar_mgr.h"
#include "bstone_logger.h"
#include "bstone_page_mgr.h"
#include "bstone_sys_polling_audio_device.h"
#include "bstone_sys_system_mgr.h"

namespace bstone {
namespace globals {

constexpr auto max_cvars = 1024;
extern CVarMgrUPtr cvar_mgr;

constexpr auto max_ccmds = 64;
extern CCmdMgrUPtr ccmd_mgr;

extern PageMgrUPtr page_mgr;

extern sys::SystemMgrUPtr sys_system_mgr;
extern sys::EventMgr* sys_event_mgr;
extern sys::VideoMgr* sys_video_mgr;
extern sys::MouseMgr* sys_mouse_mgr;
extern sys::WindowMgr* sys_window_mgr;

extern LoggerUPtr logger;

} // globals
} // bstone
