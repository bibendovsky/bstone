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
constexpr auto al_feed_con = 0xC0;


struct Instrument
{
	int m_char_;
	int c_char_;
	int m_scale_;
	int c_scale_;
	int m_attack_;
	int c_attack_;
	int m_sus_;
	int c_sus_;
	int m_wave_;
	int c_wave_;
}; // Instrument


//
// Initializes OPL3 emulator's registries.
//
bool initialize_registers(
	Opl3* opl3);

//
// Initializes OPL3 emulator instrument's registries.
//
bool set_instrument(
	Opl3* opl3,
	const Instrument& instrument);


} // adlib
} // bstone


#endif // !BSTONE_ADLIB_DECODER_INCLUDED
