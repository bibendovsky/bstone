/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#if !defined(BSTONE_SYS_SDL_PUSH_AUDIO_DEVICE_INCLUDED)
#define BSTONE_SYS_SDL_PUSH_AUDIO_DEVICE_INCLUDED

#include "bstone_sys_push_audio_device.h"
#include "bstone_sys_logger.h"

namespace bstone {
namespace sys {

PushAudioDeviceUPtr make_sdl_push_audio_device(Logger& logger, const PushAudioDeviceOpenParam& param);

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_SDL_PUSH_AUDIO_DEVICE_INCLUDED
