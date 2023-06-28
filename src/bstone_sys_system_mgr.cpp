/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_exception.h"
#include "bstone_sys_system_mgr.h"

namespace bstone {
namespace sys {

AudioMgrUPtr SystemMgr::make_audio_mgr()
BSTONE_BEGIN_FUNC_TRY
	return do_make_audio_mgr();
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

EventMgrUPtr SystemMgr::make_event_mgr()
BSTONE_BEGIN_FUNC_TRY
	return do_make_event_mgr();
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

VideoMgrUPtr SystemMgr::make_video_mgr()
BSTONE_BEGIN_FUNC_TRY
	return do_make_video_mgr();
BSTONE_END_FUNC_CATCH_ALL_THROW_NESTED

} // namespace sys
} // namespace bstone
