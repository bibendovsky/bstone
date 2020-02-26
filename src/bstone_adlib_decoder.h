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
// A base class for AdLib audio decoding.
//


#ifndef BSTONE_ADLIB_DECODER_INCLUDED
#define BSTONE_ADLIB_DECODER_INCLUDED


#include "bstone_audio_decoder.h"
#include "bstone_opl2.h"


namespace bstone
{


// A base class for AdLib audio decoding.
class AdlibDecoder :
	public AudioDecoder
{
public:
	AdlibDecoder();

	~AdlibDecoder() override;

	bool initialize(
		const void* raw_data,
		int raw_size,
		int dst_rate) override;

	void uninitialize() override;

	bool reset() override;


protected:
	Opl2 emulator_;


	void set_emulator_default_state();


private:
	void uninitialize_internal();
}; // AdlibDecoder


} // bstone


#endif // !BSTONE_ADLIB_DECODER_INCLUDED
