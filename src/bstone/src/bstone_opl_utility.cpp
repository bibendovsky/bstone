/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// OPL utility

#include "bstone_opl_utility.h"
#include "bstone_opl3.h"

namespace bstone {

void OplUtility::initialize_registers(Opl3& opl3)
{
	opl3.write(0x01, 0x20); // Set D5 ("WAVE SELECT ENABLE"), reset the other bits.
	// Reset the remaining registers.
	for (int i = 0x02; i <= 0xF5; ++i)
		opl3.write(i, 0x00);
}

void OplUtility::set_instrument(Opl3& opl3, const OplInstrument& instrument)
{
	constexpr int c = 3; // Carrier.
	constexpr int m = 0; // Modifier.
	constexpr int al_char     = 0x20;
	constexpr int al_scale    = 0x40;
	constexpr int al_attack   = 0x60;
	constexpr int al_sus      = 0x80;
	constexpr int al_wave     = 0xE0;
	constexpr int al_feed_con = 0xC0;
	opl3.write(al_effects, 0);
	opl3.write(al_char   + m, instrument.m_char);
	opl3.write(al_scale  + m, instrument.m_scale);
	opl3.write(al_attack + m, instrument.m_attack);
	opl3.write(al_sus    + m, instrument.m_sus);
	opl3.write(al_wave   + m, instrument.m_wave);
	opl3.write(al_char   + c, instrument.c_char);
	opl3.write(al_scale  + c, instrument.c_scale);
	opl3.write(al_attack + c, instrument.c_attack);
	opl3.write(al_sus    + c, instrument.c_sus);
	opl3.write(al_wave   + c, instrument.c_wave);
	opl3.write(al_feed_con, 0);
}

} // namespace bstone
