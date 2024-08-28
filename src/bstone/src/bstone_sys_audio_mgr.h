/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager.

#ifndef BSTONE_SYS_AUDIO_MGR_INCLUDED
#define BSTONE_SYS_AUDIO_MGR_INCLUDED

#include <memory>

#include "bstone_sys_polling_audio_device.h"

namespace bstone {
namespace sys {

class AudioMgr
{
public:
	AudioMgr();
	virtual ~AudioMgr();

	bool is_initialized() const noexcept;

	PollingAudioDeviceUPtr make_polling_audio_device(const PollingAudioDeviceOpenParam& param);

private:
	virtual bool do_is_initialized() const noexcept = 0;

	virtual PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) = 0;
};

// ==========================================================================

using AudioMgrUPtr = std::unique_ptr<AudioMgr>;

} // namespace sys
} // namespace bstone

#endif // BSTONE_SYS_AUDIO_MGR_INCLUDED
