/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_audio_content_mgr.h"
#include <memory>
#include <string>

namespace bstone {

class AudioExtractor
{
public:
	AudioExtractor() = default;
	virtual ~AudioExtractor() = default;

	virtual void extract_music(const std::string& dst_dir) = 0;
	virtual void extract_sfx(const std::string& dst_dir) = 0;
};

// =====================================

using AudioExtractorUPtr = std::unique_ptr<AudioExtractor>;

AudioExtractorUPtr make_audio_extractor(AudioContentMgr& audio_content_mgr);

} // namespace bstone
