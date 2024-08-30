/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// PC Speaker audio decoder.
//

#include "bstone_pc_speaker_audio_decoder.h"
#include <algorithm>
#include "bstone_assert.h"

namespace bstone
{

namespace
{

class PcSpeakerAudioDecoder final : public AudioDecoder
{
public:
	~PcSpeakerAudioDecoder() override;

	bool initialize(const AudioDecoderInitParam& param) override;
	void uninitialize() override;
	bool is_initialized() const noexcept override;

	int decode(int dst_count, std::int16_t* dst_data) override;
	bool rewind() override;

	int get_dst_length_in_samples() const noexcept override;

private:
	static constexpr auto min_src_size = 6;

	static constexpr auto command_rate = 140;
	static constexpr auto min_command = 1;
	static constexpr auto max_command = 254;
	static constexpr auto pit_clock_frequency = 1'193'180;

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

	static int make_pit_frequency(int command);
}; // PcSpeakerAudioDecoder

// --------------------------------------------------------------------------

PcSpeakerAudioDecoder::~PcSpeakerAudioDecoder() = default;

bool PcSpeakerAudioDecoder::initialize(const AudioDecoderInitParam& param)
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
	total_sample_count_ = static_cast<int>(((static_cast<long long>(commands_size_) * dst_sample_rate_) + command_rate - 1) / command_rate);
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

int PcSpeakerAudioDecoder::decode(int dst_count, std::int16_t* dst_data)
{
	if (!is_initialized_ || is_finished_)
	{
		return 0;
	}

	auto sample_offset = 0;

	while (true)
	{
		if (sample_offset >= dst_count)
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

		const auto sample = (pit_signal_level_ == 0 ? -32'768 : 32'767); // [0, 1] => [-32768, +32767]
		dst_data[sample_offset] = static_cast<std::int16_t>(sample);
		sample_offset += 1;
		command_counter_ += command_rate;
		pit_counter_ += pit_counter_step_;
	}

	return sample_offset;
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

int PcSpeakerAudioDecoder::make_pit_frequency(int command)
{
	BSTONE_ASSERT(command >= min_command && command <= max_command);

	const auto divisor = command * 60;
	const auto pit_frequency = pit_clock_frequency / divisor;
	return pit_frequency;
}

} // namespace

// ==========================================================================

AudioDecoderUPtr make_pc_speaker_audio_decoder()
{
	return std::make_unique<PcSpeakerAudioDecoder>();
}

} // bstone
