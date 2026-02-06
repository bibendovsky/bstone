/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_AUDIO_CONTENT_MGR_INCLUDED
#define BSTONE_AUDIO_CONTENT_MGR_INCLUDED

#include "audio.h"
#include "bstone_vswap.h"
#include <memory>

namespace bstone {

class AudioContentMgr
{
public:
	AudioContentMgr() = default;
	virtual ~AudioContentMgr() = default;

	virtual AudioSfxType get_sfx_type() const = 0;
	virtual void set_sfx_type(AudioSfxType sfx_type) = 0;
	virtual void set_is_sfx_digitized(bool is_digitized) = 0;
	virtual int get_chunk_count() const = 0;
	virtual const AudioChunk& get_chunk(int chunk_number) const = 0;
	virtual const AudioChunk& get_sfx_chunk(int chunk_number) const = 0;
	virtual int get_sfx_priority(int chunk_number) const = 0;
	virtual const AudioChunk& get_adlib_music_chunk(int chunk_number) const = 0;
};

// =====================================

using AudioContentMgrUPtr = std::unique_ptr<AudioContentMgr>;

AudioContentMgrUPtr make_audio_content_mgr(Vswap& vswap);

} // namespace bstone

#endif // BSTONE_AUDIO_CONTENT_MGR_INCLUDED
