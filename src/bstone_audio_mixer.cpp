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


#include "bstone_oal_audio_mixer.h"
#include "bstone_sdl2_audio_mixer.h"

#include "bstone_exception.h"
#include "bstone_logger.h"


namespace bstone
{

class MakeAudioMixerException :
	public Exception
{
public:
	explicit MakeAudioMixerException(
		const std::string& message) :
		Exception{"[MAKE_AUDIO_MIXER] " + message}
	{
	}
}; // MakeAudioMixerException


AudioMixerUPtr make_audio_mixer(
	AudioDriverType audio_driver_type,
	MtTaskMgr* const mt_task_manager)
{
	switch (audio_driver_type)
	{
		case AudioDriverType::r2_sdl:
			return std::make_unique<Sdl2AudioMixer>(mt_task_manager);

		case AudioDriverType::r3_openal:
			return std::make_unique<OalAudioMixer>(mt_task_manager);

		default:
			throw MakeAudioMixerException{"Unsupported driver type."};
	}
}


} // bstone
