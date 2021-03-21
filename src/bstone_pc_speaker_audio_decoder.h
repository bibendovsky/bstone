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
// PC Speaker audio decoder.
//


#ifndef BSTONE_PC_SPEAKER_AUDIO_DECODER_INCLUDED
#define BSTONE_PC_SPEAKER_AUDIO_DECODER_INCLUDED


#include <cassert>

#include "bstone_audio_decoder.h"


namespace bstone
{


// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

class PcSpeakerAudioDecoder :
	public AudioDecoder
{
public:
	bool initialize(
		const AudioDecoderInitParam& param) override;

	void uninitialize() override;

	bool is_initialized() const noexcept override;

	int decode(
		int dst_count,
		std::int16_t* dst_data) override;

	int decode(
		int dst_count,
		float* dst_data) override;

	bool rewind() override;

	int get_dst_length_in_samples() const noexcept override;

	bool set_resampling(
		AudioDecoderInterpolationType interpolation_type,
		bool lpf,
		bool lpf_flush_samples) override;


private:
	static constexpr auto min_src_size = 6;

	static constexpr auto command_rate = 140;
	static constexpr auto min_command = 1;
	static constexpr auto max_command = 254;
	static constexpr auto pit_clock_frequency = 1'193'180;


	struct OutputSamples
	{
		bool is_s16;

		union
		{
			std::int16_t* s16;
			float* f32;
		};
	}; // OutputSamples


	int dst_sample_rate_{};
	const std::uint8_t* commands_{};
	int commands_size_{};

	int total_sample_count_{};
	int command_offset_{};
	int last_command_{};
	int pit_signal_level_{};
	int pit_counter_step_{};
	int pit_counter_{};
	int command_counter_{};
	bool is_initialized_{};
	bool is_finished_{};


	static int make_pit_frequency(
		int command);

	int decode(
		const OutputSamples& output_samples,
		int max_samples);
}; // PcSpeakerAudioDecoder

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


} // bstone


#endif // !BSTONE_PC_SPEAKER_AUDIO_DECODER_INCLUDED
