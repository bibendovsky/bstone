/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager stub.

#ifndef BSTONE_SYS_AUDIO_MGR_NULL_INCLUDED
#define BSTONE_SYS_AUDIO_MGR_NULL_INCLUDED

#include "bstone_sys_audio_mgr.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

AudioMgrUPtr make_null_audio_mgr(Logger& logger);

} // namespace bstone::sys

#endif // BSTONE_SYS_AUDIO_MGR_NULL_INCLUDED
