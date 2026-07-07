/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// OPL music decoder

#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_opl_emulator.h"
#include "bstone_opl_utility.h"
#include <algorithm>

namespace bstone {

namespace {

class OplMusicDecoder final : public AudioDecoder
{
public:
	explicit OplMusicDecoder(OplEmulatorType opl_emulator_type);
	~OplMusicDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int frame_count) override;
	bool rewind() override;

private:
	inline static constexpr int tick_rate = 700;

	bool is_initialized_{};
	int commands_count_{};
	int command_offset_{};
	int samples_per_tick_{};
	int frames_left_{};
	const char* error_message_{};
	OplEmulatorUPtr emulator_{};
	MemoryBinaryReader reader_{};

	void impl_terminate();
	int impl_get_channel_count() const;
	void set_error_message(const char* error_message);
};

// -------------------------------------

OplMusicDecoder::OplMusicDecoder(OplEmulatorType opl_emulator_type)
	:
	emulator_{make_opl_emulator(opl_emulator_type)}
{}

bool OplMusicDecoder::initialize(const AudioDecoderInitParam& param)
{
	terminate();
	if (emulator_ == nullptr)
	{
		set_error_message("Unknown emulator type.");
		return false;
	}
	if (param.src_raw_data == nullptr)
	{
		set_error_message("No source data.");
		return false;
	}
	if (param.src_raw_size < 0)
	{
		set_error_message("Invalid source size.");
		return false;
	}
	if (param.dst_rate <= 0)
	{
		set_error_message("Invalid sample rate.");
		return false;
	}
	emulator_->initialize(OplEmulatorInitParam{.sample_rate = param.dst_rate});
	reader_ = MemoryBinaryReader{param.src_raw_data, param.src_raw_size};
	if (!reader_.can_read_x16())
	{
		set_error_message("Source data too small.");
		return false;
	}
	const int commands_size = reader_.read_u16_le();
	if ((commands_size % 4) != 0)
	{
		set_error_message("Misaligned source data.");
		return false;
	}
	if (!reader_.can_read_n(commands_size))
	{
		set_error_message("Command count mismatch.");
		return false;
	}
	command_offset_ = 0;
	commands_count_ = commands_size / 4;
	samples_per_tick_ = 0;
	frames_left_ = 0;
	int ticks_count = 0;
	for (int i = 0; i < commands_count_; ++i)
	{
		reader_.skip(2);
		ticks_count += reader_.read_u16_le();
	}
	reader_.set_position(2);
	is_initialized_ = true;
	return true;
}

void OplMusicDecoder::terminate()
{
	impl_terminate();
}

bool OplMusicDecoder::is_initialized() const
{
	return is_initialized_;
}

const char* OplMusicDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int OplMusicDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return impl_get_channel_count();
}

bool OplMusicDecoder::rewind()
{
	BSTONE_ASSERT(is_initialized());
	OplUtility::initialize_registers(*emulator_);
	reader_.set_position(2);
	command_offset_ = 0;
	frames_left_ = 0;
	samples_per_tick_ = 0;
	return true;
}

int OplMusicDecoder::decode_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(frame_count >= 0);
	if (command_offset_ == commands_count_ && frames_left_ == 0)
		return 0;
	int decoded_frame_count = 0;
	int frame_offset = 0;
	int frames_left = frame_count;
	for (bool quit = false; !quit; )
	{
		if (frames_left_ > 0)
		{
			const int generated_frame_count = std::min(frames_left, frames_left_);
			emulator_->generate_frames(&samples[frame_offset * impl_get_channel_count()], generated_frame_count);
			frame_offset += generated_frame_count;
			frames_left -= generated_frame_count;
			frames_left_ -= generated_frame_count;
			decoded_frame_count += generated_frame_count;
		}
		else
		{
			int delay = 0;
			while (command_offset_ < commands_count_ && delay == 0)
			{
				const int command_address = reader_.read_u8();
				const int command_value = reader_.read_u8();
				delay = reader_.read_u16_le();
				emulator_->write_deferred(command_address, command_value);
				++command_offset_;
			}
			if (delay > 0)
			{
				samples_per_tick_ += delay * emulator_->get_sample_rate();
				frames_left_ = samples_per_tick_ / tick_rate;
				samples_per_tick_ %= tick_rate;
			}
		}
		quit = (command_offset_ == commands_count_ && frames_left_ == 0) || frames_left == 0;
	}
	return decoded_frame_count;
}

void OplMusicDecoder::impl_terminate()
{
	is_initialized_ = false;
}

int OplMusicDecoder::impl_get_channel_count() const
{
	return emulator_->get_channel_count();
}

void OplMusicDecoder::set_error_message(const char* error_message)
{
	error_message_ = error_message;
}

} // namespace

// =====================================

AudioDecoderUPtr make_opl_music_audio_decoder(OplEmulatorType opl_emulator_type)
{
	return std::make_unique<OplMusicDecoder>(opl_emulator_type);
}

} // namespace bstone
