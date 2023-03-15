/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_sys_audio_mgr.h"

namespace bstone {
namespace sys {

PushAudioDeviceUPtr AudioMgr::make_audio_device(const PushAudioDeviceOpenParam& param)
{
	return do_make_audio_device(param);
}

} // namespace sys
} // namespace bstone
