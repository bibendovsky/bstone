/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// A base class for AdLib decoder.
//


#include "bstone_adlib_decoder.h"


namespace bstone
{


AdlibDecoder::AdlibDecoder()
	:
	emulator_{}
{
}

AdlibDecoder::~AdlibDecoder()
{
}

bool AdlibDecoder::initialize(
	const void* const raw_data,
	const int raw_size,
	const int dst_rate)
{
	uninitialize();

	if (!AudioDecoder::initialize(raw_data, raw_size, dst_rate))
	{
		return false;
	}

	emulator_.initialize(dst_rate);
	set_emulator_default_state();

	return true;
}

void AdlibDecoder::uninitialize()
{
	uninitialize_internal();

	AudioDecoder::uninitialize();
}

bool AdlibDecoder::reset()
{
	if (!is_initialized())
	{
		return false;
	}

	emulator_.initialize(get_dst_rate());
	set_emulator_default_state();

	return true;
}

void AdlibDecoder::set_emulator_default_state()
{
	for (int i = 1; i <= 0xF5; ++i)
	{
		emulator_.write(i, 0x00);
	}

	emulator_.write(0x01, 0x20);
	emulator_.write(0x08, 0x00);
}

void AdlibDecoder::uninitialize_internal()
{
	emulator_.uninitialize();
}


} // bstone
