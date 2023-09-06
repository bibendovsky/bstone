/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// A stub for DOSBox OPL2 emulator.

#include <cassert>

#include <algorithm>

#include "mixer.h"

void MixerChannel::AddSamples_m32(Bitu len, const Bit32s* data)
{
	for (auto i = decltype(len){}; i < len; ++i)
	{
		auto value = data[i];
		value = std::min(value, get_max_sample_value());
		value = std::max(value, get_min_sample_value());
		buffer_[i] = static_cast<Bit16s>(value);
	}
}

void MixerChannel::AddSamples_s32(Bitu, const Bit32s*)
{
	assert(!"AddSamples_s32 not implemented.");
}

// Sets a buffer to write data to.
void MixerChannel::set_buffer(Bit16s* buffer)
{
	buffer_ = buffer;
}

std::int32_t MixerChannel::get_min_sample_value()
{
	return -32768;
}

std::int32_t MixerChannel::get_max_sample_value()
{
	return 32767;
}
