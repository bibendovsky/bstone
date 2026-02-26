/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-late
*/

// A stub for DOSBox OPL2 emulator

#include "mixer.h"
#include "bstone_assert.h"
#include <climits>
#include <algorithm>

void MixerChannel::AddSamples_m32(Bitu len, const Bit32s* data)
{
	for (Bitu i = 0; i < len; ++i)
	{
		buffer_[i] = static_cast<Bit16s>(std::clamp(data[i], get_min_sample_value(), get_max_sample_value()));
	}
}

void MixerChannel::AddSamples_s32([[maybe_unused]] Bitu len, [[maybe_unused]] const Bit32s* data)
{
	BSTONE_ASSERT(false && "AddSamples_s32 not implemented.");
}

// Sets a buffer to write data to.
void MixerChannel::set_buffer(Bit16s* buffer)
{
	buffer_ = buffer;
}

std::int32_t MixerChannel::get_min_sample_value()
{
	return INT16_MIN;
}

std::int32_t MixerChannel::get_max_sample_value()
{
	return INT16_MAX;
}
