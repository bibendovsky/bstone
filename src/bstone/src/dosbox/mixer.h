/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A stub for DOSBox OPL2 emulator.

#ifndef BSTONE_DOSBOX_MIXER_INCLUDED
#define BSTONE_DOSBOX_MIXER_INCLUDED

#include "dosbox.h"

class MixerChannel
{
public:
	void AddSamples_m32(Bitu len, const Bit32s* data);
	void AddSamples_s32(Bitu len, const Bit32s* data);

	// Sets a buffer to write data to.
	void set_buffer(Bit16s* buffer);

	static Bit32s get_min_sample_value();
	static Bit32s get_max_sample_value();

private:
	Bit16s* buffer_{};
};

struct MixerObject {};

#endif // BSTONE_DOSBOX_MIXER_INCLUDED
