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
// A PCM decoder for digitized sounds.
//


#ifndef BSTONE_PCM_DECODER_INCLUDED
#define BSTONE_PCM_DECODER_INCLUDED


#include "bstone_audio_decoder.h"


namespace bstone
{


// A PCM decoder for digitized sounds.
class PcmDecoder final :
	public AudioDecoder
{
public:
	PcmDecoder();

	~PcmDecoder() override;

	bool initialize(
		const void* const src_raw_data,
		const int src_raw_size,
		const int dst_rate) override;

	void uninitialize() override;

	int decode(
		const int dst_count,
		std::int16_t* const dst_data) override;

	bool reset() override;

	AudioDecoder* clone() override;

	// Return an input sample rate.
	static int get_src_rate();

	// Returns a minimum output sample rate.
	static int get_min_dst_rate();


private:
	std::int64_t offset_;
	std::int16_t last_sample_;
	std::int64_t dst_count_;
	std::int64_t dst_ratio_;
	double alpha_;
	double one_minus_alpha_;


	void uninitialize_internal();

	static std::int16_t pcm8_to_pcm16(
		const std::uint8_t sample);
}; // PcmDecoder


} // bstone


#endif // !BSTONE_PCM_DECODER_INCLUDED
