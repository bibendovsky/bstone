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


#include "bstone_pc_speaker_audio_decoder.h"

#include <cassert>

#include <algorithm>


namespace bstone
{


bool PcSpeakerAudioDecoder::initialize(
	const AudioDecoderInitParam& param)
{
	if (!param.src_raw_data_)
	{
		return false;
	}

	if (param.src_raw_size_ < min_src_size)
	{
		return false;
	}

	if (param.dst_rate_ <= command_rate)
	{
		return false;
	}

	const auto data_size = static_cast<int>(*reinterpret_cast<const std::uint32_t*>(param.src_raw_data_));

	dst_sample_rate_ = param.dst_rate_;
	commands_ = static_cast<const std::uint8_t*>(param.src_raw_data_) + min_src_size;
	commands_size_ = data_size;

	total_sample_count_ = static_cast<int>(
		((static_cast<long long>(commands_size_) * dst_sample_rate_) + command_rate - 1) / command_rate
	);

	command_offset_ = 0;
	last_command_ = 0;
	pit_signal_level_ = 0;
	pit_counter_step_ = 0;
	pit_counter_ = 0;
	command_counter_ = dst_sample_rate_;
	is_finished_ = false;
	is_initialized_ = true;

	return true;
}

void PcSpeakerAudioDecoder::uninitialize()
{
	dst_sample_rate_ = 0;
	commands_ = nullptr;
	commands_size_ = 0;

	total_sample_count_ = 0;
	command_offset_ = 0;
	last_command_ = 0;
	pit_signal_level_ = 0;
	pit_counter_step_ = 0;
	pit_counter_ = 0;
	command_counter_ = 0;
	is_initialized_ = false;
	is_finished_ = false;
}

bool PcSpeakerAudioDecoder::is_initialized() const noexcept
{
	return is_initialized_;
}

int PcSpeakerAudioDecoder::decode(
	int dst_count,
	std::int16_t* dst_data)
{
	auto output_samples = OutputSamples{};
	output_samples.is_s16 = true;
	output_samples.s16 = dst_data;

	return decode(output_samples, dst_count);
}

int PcSpeakerAudioDecoder::decode(
	int dst_count,
	float* dst_data)
{
	auto output_samples = OutputSamples{};
	output_samples.is_s16 = false;
	output_samples.f32 = dst_data;

	return decode(output_samples, dst_count);
}

bool PcSpeakerAudioDecoder::rewind()
{
	if (!is_initialized_)
	{
		return false;
	}

	command_offset_ = 0;
	is_finished_ = false;

	return true;
}

int PcSpeakerAudioDecoder::get_dst_length_in_samples() const noexcept
{
	return total_sample_count_;
}

bool PcSpeakerAudioDecoder::set_resampling(
	AudioDecoderInterpolationType,
	bool,
	bool)
{
	return true;
}

int PcSpeakerAudioDecoder::make_pit_frequency(
	int command)
{
	assert(command >= min_command && command <= max_command);

	const auto divisor = command * 60;
	const auto pit_frequency = pit_clock_frequency / divisor;

	return pit_frequency;
}

int PcSpeakerAudioDecoder::decode(
	const OutputSamples& output_samples,
	int max_samples)
{
	if (!is_initialized_ || is_finished_)
	{
		return 0;
	}

	auto sample_offset = 0;

	while (true)
	{
		if (sample_offset >= max_samples)
		{
			break;
		}

		if (command_counter_ >= dst_sample_rate_)
		{
			command_counter_ -= dst_sample_rate_;

			if (command_offset_ >= commands_size_)
			{
				is_finished_ = true;
				break;
			}

			const auto command = static_cast<int>(commands_[command_offset_]);
			command_offset_ += 1;

			if (last_command_ != command)
			{
				pit_counter_ = 0;

				if (command != 0)
				{
					pit_counter_step_ = 2 * make_pit_frequency(command);
					pit_signal_level_ = 1;
				}
				else
				{
					pit_counter_step_ = 0;
					pit_signal_level_ = 0;
				}
			}

			last_command_ = command;
		}

		while (pit_counter_ > 0 && pit_counter_ >= dst_sample_rate_)
		{
			pit_counter_ -= dst_sample_rate_;
			pit_signal_level_ = 1 - pit_signal_level_;
		}

		if (output_samples.is_s16)
		{
			const auto sample = (pit_signal_level_ == 0 ? -32'768 : 32'767); // [0, 1] => [-32768, +32767]
			output_samples.s16[sample_offset] = static_cast<std::int16_t>(sample);
		}
		else
		{
			const auto sample = static_cast<float>((2 * pit_signal_level_) - 1); // [0, 1] => [-1, +1]
			output_samples.f32[sample_offset] = sample;
		}

		sample_offset += 1;

		command_counter_ += command_rate;
		pit_counter_ += pit_counter_step_;
	}

	return sample_offset;
}


} // bstone
