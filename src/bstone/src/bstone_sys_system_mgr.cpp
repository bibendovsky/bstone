/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// System manager.

#include "bstone_sys_system_mgr.h"

namespace bstone {
namespace sys {

SystemMgr::SystemMgr() = default;

SystemMgr::~SystemMgr() = default;

AudioMgr& SystemMgr::get_audio_mgr()
{
	return do_get_audio_mgr();
}

EventMgr& SystemMgr::get_event_mgr()
{
	return do_get_event_mgr();
}

VideoMgr& SystemMgr::get_video_mgr()
{
	return do_get_video_mgr();
}

} // namespace sys
} // namespace bstone
