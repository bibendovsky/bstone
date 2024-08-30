/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#ifndef BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL2_INCLUDED
#define BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL2_INCLUDED

#include "bstone_sys_polling_audio_device.h"
#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

PollingAudioDeviceUPtr make_sdl2_polling_audio_device(Logger& logger, const PollingAudioDeviceOpenParam& param);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL2_INCLUDED
