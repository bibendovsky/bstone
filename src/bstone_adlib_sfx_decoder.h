/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// A decoder for AdLib sound effects.
//


#ifndef BSTONE_ADLIB_SFX_DECODER_INCLUDED
#define BSTONE_ADLIB_SFX_DECODER_INCLUDED


#include <vector>
#include "bstone_adlib_decoder.h"
#include "bstone_memory_binary_reader.h"


namespace bstone
{


// A decoder for AdLib sound effects.
class AdlibSfxDecoder final :
	public AdlibDecoder
{
public:
	AdlibSfxDecoder();

	~AdlibSfxDecoder() override;

	bool initialize(
		const void* const raw_data,
		const int raw_size,
		const int dst_rate) override;

	void uninitialize() override;

	bool reset() override;

	AudioDecoder* clone() override;

	int decode(
		const int dst_count,
		std::int16_t* const dst_data) override;

	// Returns a number of calls per second of
	// original interrupt routine.
	static int get_tick_rate();


private:
	static const auto al_char = 0x20;
	static const auto al_scale = 0x40;
	static const auto al_attack = 0x60;
	static const auto al_sus = 0x80;
	static const auto al_wave = 0xE0;
	static const auto al_freq_l = 0xA0;
	static const auto al_freq_h = 0xB0;
	static const auto al_feed_con = 0xC0;


	struct Instrument final
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


		void reset();
	}; // Instrument


	MemoryBinaryReader reader_;
	Instrument instrument_;
	int commands_count_;
	int command_index_;
	int samples_per_tick_;
	int remains_count_;
	int hf_;


	void uninitialize_internal();

	void initialize_instrument();

	// Returns an original size of an AdLibSound structure.
	static int get_header_size();
}; // AdlibDecoder


} // bstone


#endif // !BSTONE_ADLIB_SFX_DECODER_INCLUDED
