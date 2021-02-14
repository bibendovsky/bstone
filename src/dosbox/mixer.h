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


//
// A stub for DOSBox OPL2 emulator.
//


#ifndef BSTONE_DOSBOX_MIXER_INCLUDED
#define BSTONE_DOSBOX_MIXER_INCLUDED


#include <algorithm>

#include "dosbox.h"


typedef void(*MIXER_Handler)(std::uintptr_t len);


class MixerChannel
{
public:
	MixerChannel()
	{
		set_buffer(nullptr);
		set_scale(get_default_scale());
	}

	void AddSamples_m32(std::uintptr_t count, const std::int32_t* buffer)
	{
		if (count < 1)
			return;

		if (!buffer)
			return;

		assert(buffer_);

		if (!buffer_)
			return;

		for (std::uintptr_t i = 0; i < count; ++i)
		{
			std::int32_t value = scale_ * buffer[i];

			value = std::min(value, get_max_sample_value());
			value = std::max(value, get_min_sample_value());

			buffer_[i] = static_cast<std::int16_t>(value);
		}
	}

	void AddSamples_s32(std::uintptr_t, const std::int32_t*)
	{
		assert(!"AddSamples_s32 not implemented.");
	}

	// Sets a buffer to write data to.
	void set_buffer(std::int16_t* buffer)
	{
		buffer_ = buffer;
	}

	// Sets a scale for output samples.
	void set_scale(int new_scale)
	{
		scale_ = std::max(new_scale, get_min_scale());
	}

	static int get_min_scale()
	{
		return 1;
	}

	static int get_default_scale()
	{
		return get_min_scale();
	}

	static std::int32_t get_min_sample_value()
	{
		return -32768;
	}

	static std::int32_t get_max_sample_value()
	{
		return 32767;
	}

private:
	std::int16_t* buffer_;
	int scale_;
}; // MixerChannel


struct MixerObject
{
}; // MixerObject


#endif // BSTONE_DOSBOX_MIXER_INCLUDED
