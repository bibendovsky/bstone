/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// OPL utility

#ifndef BSTONE_OPL_UTILITY_INCLUDED
#define BSTONE_OPL_UTILITY_INCLUDED

#include "bstone_opl_emulator.h"

namespace bstone {

struct OplInstrument
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
};

class OplUtility
{
public:
	inline static constexpr int al_freq_l = 0xA0;
	inline static constexpr int al_freq_h = 0xB0;
	inline static constexpr int al_effects = 0xBD;

	static void initialize_registers(OplEmulator& opl3);
	static void set_instrument(OplEmulator& opl3, const OplInstrument& instrument);
};

} // namespace bstone

#endif // BSTONE_OPL_UTILITY_INCLUDED
