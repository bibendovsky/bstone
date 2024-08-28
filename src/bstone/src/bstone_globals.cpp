/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_globals.h"

namespace bstone {
namespace globals {

CVarMgrUPtr cvar_mgr{};
CCmdMgrUPtr ccmd_mgr{};
PageMgrUPtr page_mgr{};

sys::SystemMgrUPtr sys_system_mgr{};
sys::EventMgr* sys_event_mgr{};
sys::VideoMgr* sys_video_mgr{};
sys::MouseMgr* sys_mouse_mgr{};
sys::WindowMgr* sys_window_mgr{};

LoggerUPtr logger{};

} // globals
} // bstone
