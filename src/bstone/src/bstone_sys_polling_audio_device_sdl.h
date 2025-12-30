/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Polling audio device (SDL)

#ifndef BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL_INCLUDED
#define BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL_INCLUDED

#include "bstone_sys_polling_audio_device.h"
#include "bstone_sys_logger.h"

namespace bstone::sys {

PollingAudioDeviceUPtr make_polling_audio_device_sdl(Logger& logger, const PollingAudioDeviceOpenParam& param);

} // namespace bstone::sys

#endif // BSTONE_SYS_POLLING_AUDIO_DEVICE_SDL_INCLUDED
