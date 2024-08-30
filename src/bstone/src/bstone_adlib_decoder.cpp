/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// AdLib audio decoder common tasks.
//

#include "bstone_adlib_decoder.h"
#include "bstone_opl3.h"

namespace bstone
{
namespace adlib
{

bool initialize_registers(Opl3* opl3)
{
	if (!opl3)
	{
		return false;
	}

	// Zero all the registers.
	for (int i = 1; i <= 0xF5; ++i)
	{
		opl3->write(i, 0x00);
	}

	opl3->write(0x01, 0x20); // Set WSE=1
	opl3->write(0x08, 0x00); // Set CSM=0 & SEL=0

	constexpr auto al_effects = 0xBD;
	opl3->write(al_effects, 0x00);
	static const auto zero_instrument = Instrument{};
	return set_instrument(opl3, zero_instrument);
}

bool set_instrument(Opl3* opl3, const Instrument& instrument)
{
	// Carrier.
	constexpr auto c = 3;

	// Modifier.
	constexpr auto m = 0;

	// Operator stuff.
	//

	constexpr auto al_char = 0x20;
	constexpr auto al_scale = 0x40;
	constexpr auto al_attack = 0x60;
	constexpr auto al_sus = 0x80;
	constexpr auto al_wave = 0xE0;
	constexpr auto al_feed_con = 0xC0;

	opl3->write(0xBD, 0);
	opl3->write(m + al_char, instrument.m_char);
	opl3->write(m + al_scale, instrument.m_scale);
	opl3->write(m + al_attack, instrument.m_attack);
	opl3->write(m + al_sus, instrument.m_sus);
	opl3->write(m + al_wave, instrument.m_wave);
	opl3->write(c + al_char, instrument.c_char);
	opl3->write(c + al_scale, instrument.c_scale);
	opl3->write(c + al_attack, instrument.c_attack);
	opl3->write(c + al_sus, instrument.c_sus);
	opl3->write(c + al_wave, instrument.c_wave);
	opl3->write(al_feed_con, 0);
	return true;
}

} // adlib
} // bstone
