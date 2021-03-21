/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#ifndef BSTONE_AUDIO_CONTENT_MGR_INCLUDED
#define BSTONE_AUDIO_CONTENT_MGR_INCLUDED


#include <cstdint>

#include <memory>

#include "audio.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class AudioContentMgr
{
public:
	AudioContentMgr() noexcept;

	virtual ~AudioContentMgr();


	virtual AudioSfxType get_sfx_type() const noexcept = 0;

	virtual void set_sfx_type(
		AudioSfxType sfx_type) = 0;


	virtual void set_is_sfx_digitized(
		bool is_allow) noexcept = 0;


	virtual int get_chunk_count() const noexcept = 0;

	virtual const AudioChunk& get_chunk(
		int chunk_number) const = 0;


	virtual const AudioChunk& get_sfx_chunk(
		int sfx_chunk_number) const = 0;

	virtual int get_sfx_priority(
		int sfx_chunk_number) const = 0;


	virtual const AudioChunk& get_adlib_music_chunk(
		int adlib_music_chunk_number) const = 0;
}; // AudioContentMgr

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

using AudioContentMgrUPtr = std::unique_ptr<AudioContentMgr>;

AudioContentMgrUPtr make_audio_content_mgr();

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_AUDIO_CONTENT_MGR_INCLUDED
