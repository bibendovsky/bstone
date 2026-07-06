/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// PC Speaker audio decoder

#include "bstone_pc_speaker_audio_decoder.h"
#include "bstone_assert.h"
#include "bstone_endian.h"
#include <algorithm>

namespace bstone {

namespace {

class PcSpeakerAudioDecoder final : public AudioDecoder
{
public:
	~PcSpeakerAudioDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_total_frames() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int frame_count) override;
	bool rewind() override;

private:
	inline static constexpr int channel_count = 1;
	inline static constexpr int min_src_size = 6;

	inline static constexpr int command_rate = 140;
	[[maybe_unused]] static constexpr int min_command = 1;
	[[maybe_unused]] static constexpr int max_command = 254;
	inline static constexpr int pit_clock_frequency = 1'193'180;

	const char* error_message_{};
	int dst_sample_rate_{};
	const std::uint8_t* commands_{};
	int commands_size_{};
	int total_frames_{};
	int command_offset_{};
	int last_command_{};
	int pit_signal_level_{};
	int pit_counter_step_{};
	int pit_counter_{};
	int command_counter_{};
	bool is_initialized_{};
	bool is_finished_{};

	static int make_pit_frequency(int command);
	void set_error_message(const char* error_message);
};

// -------------------------------------

bool PcSpeakerAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	if (param.src_raw_data == nullptr)
	{
		set_error_message("No source data.");
		return false;
	}
	if (param.src_raw_size < min_src_size)
	{
		set_error_message("INvalid source size.");
		return false;
	}
	if (param.dst_rate <= command_rate)
	{
		set_error_message("Sample rate too small.");
		return false;
	}
	const int data_size = static_cast<int>(endian::read_u32_le(param.src_raw_data));
	dst_sample_rate_ = param.dst_rate;
	commands_ = static_cast<const std::uint8_t*>(param.src_raw_data) + min_src_size;
	commands_size_ = data_size;
	total_frames_ = static_cast<int>(((static_cast<long long>(commands_size_) * dst_sample_rate_) + command_rate - 1) / command_rate);
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

void PcSpeakerAudioDecoder::terminate()
{
	is_initialized_ = false;
}

bool PcSpeakerAudioDecoder::is_initialized() const
{
	return is_initialized_;
}

const char* PcSpeakerAudioDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int PcSpeakerAudioDecoder::get_total_frames() const
{
	BSTONE_ASSERT(is_initialized());
	return total_frames_;
}

int PcSpeakerAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

int PcSpeakerAudioDecoder::decode_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
	if (is_finished_)
		return 0;
	int frame_offset = 0;
	for (;;)
	{
		if (frame_offset >= frame_count)
			break;
		if (command_counter_ >= dst_sample_rate_)
		{
			command_counter_ -= dst_sample_rate_;
			if (command_offset_ >= commands_size_)
			{
				is_finished_ = true;
				break;
			}
			const int command = commands_[command_offset_];
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
		const float sample = pit_signal_level_ == 0 ? -1.0F : 1.0F;
		samples[frame_offset] = sample;
		++frame_offset;
		command_counter_ += command_rate;
		pit_counter_ += pit_counter_step_;
	}
	return frame_offset;
}

bool PcSpeakerAudioDecoder::rewind()
{
	BSTONE_ASSERT(is_initialized());
	command_offset_ = 0;
	is_finished_ = false;
	return true;
}

int PcSpeakerAudioDecoder::make_pit_frequency(int command)
{
	BSTONE_ASSERT(command >= min_command && command <= max_command);
	const int divisor = command * 60;
	const int pit_frequency = pit_clock_frequency / divisor;
	return pit_frequency;
}

void PcSpeakerAudioDecoder::set_error_message(const char* error_message)
{
	error_message_ = error_message;
}

} // namespace

// =====================================

AudioDecoderUPtr make_pc_speaker_audio_decoder()
{
	return std::make_unique<PcSpeakerAudioDecoder>();
}

} // namespace bstone
