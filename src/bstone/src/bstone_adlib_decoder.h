/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// AdLib audio decoder common tasks.
//

#ifndef BSTONE_ADLIB_DECODER_INCLUDED
#define BSTONE_ADLIB_DECODER_INCLUDED

namespace bstone
{

class Opl3;

namespace adlib
{

// Channel stuff.
//

constexpr auto al_freq_l = 0xA0;
constexpr auto al_freq_h = 0xB0;

struct Instrument
{
	int m_char;
	int c_char;
	int m_scale;
	int c_scale;
	int m_attack;
	int c_attack;
	int m_sus;
	int c_sus;
	int m_wave;
	int c_wave;
}; // Instrument

//
// Initializes OPL3 emulator's registries.
//
bool initialize_registers(Opl3* opl3);

//
// Initializes OPL3 emulator instrument's registries.
//
bool set_instrument(Opl3* opl3, const Instrument& instrument);

} // adlib
} // bstone

#endif // !BSTONE_ADLIB_DECODER_INCLUDED
