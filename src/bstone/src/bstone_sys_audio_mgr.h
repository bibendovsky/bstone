/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Audio manager.

#ifndef BSTONE_SYS_AUDIO_MGR_INCLUDED
#define BSTONE_SYS_AUDIO_MGR_INCLUDED

#include "bstone_sys_polling_audio_device.h"
#include <memory>

namespace bstone::sys {

class AudioMgr
{
public:
	AudioMgr() = default;
	virtual ~AudioMgr() = default;

	bool is_initialized() const;
	PollingAudioDeviceUPtr make_polling_audio_device(const PollingAudioDeviceOpenParam& param);

private:
	virtual bool do_is_initialized() const = 0;
	virtual PollingAudioDeviceUPtr do_make_polling_audio_device(const PollingAudioDeviceOpenParam& param) = 0;
};

// ======================================

using AudioMgrUPtr = std::unique_ptr<AudioMgr>;

} // namespace bstone::sys

#endif // BSTONE_SYS_AUDIO_MGR_INCLUDED
