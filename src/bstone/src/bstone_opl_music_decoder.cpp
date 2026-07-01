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
	void uninitialize() override;
	bool is_initialized() const override;
	int decode(int dst_count, float* dst_samples) override;
	bool rewind() override;
	int get_dst_length_in_samples() const override;
	int get_channel_count() const override;

private:
	inline static constexpr int tick_rate = 700;

	OplEmulatorUPtr emulator_{};

	bool is_initialized_{};

	MemoryBinaryReader reader_{};
	int commands_count_{};
	int command_index_{};
	int samples_per_tick_{};
	int remains_count_{};
	int dst_length_in_samples_{};

	void impl_uninitialize();
	int impl_get_channel_count() const;
};

// -------------------------------------

OplMusicDecoder::OplMusicDecoder(OplEmulatorType opl_emulator_type)
	:
	emulator_{make_opl_emulator(opl_emulator_type)}
{}

bool OplMusicDecoder::initialize(const AudioDecoderInitParam& param)
{
	uninitialize();
	if (emulator_ == nullptr)
		return false;
	if (param.src_raw_data == nullptr)
		return false;
	if (param.src_raw_size < 0)
		return false;
	if (param.dst_rate <= 0)
		return false;
	emulator_->initialize(OplEmulatorInitParam{.sample_rate = param.dst_rate});
	reader_ = MemoryBinaryReader{param.src_raw_data, param.src_raw_size};
	if (!reader_.can_read_x16())
		return false;
	const int commands_size = reader_.read_u16_le();
	if ((commands_size % 4) != 0)
		return false;
	if (!reader_.can_read_n(commands_size))
		return false;
	command_index_ = 0;
	commands_count_ = commands_size / 4;
	samples_per_tick_ = 0;
	remains_count_ = 0;
	int ticks_count = 0;
	for (int i = 0; i < commands_count_; ++i)
	{
		reader_.skip(2);
		ticks_count += reader_.read_u16_le();
	}
	dst_length_in_samples_ = static_cast<int>(static_cast<long long>(ticks_count) * emulator_->get_sample_rate() / tick_rate);
	reader_.set_position(2);
	is_initialized_ = true;
	return true;
}

bool OplMusicDecoder::is_initialized() const
{
	return is_initialized_;
}

void OplMusicDecoder::uninitialize()
{
	impl_uninitialize();
}

bool OplMusicDecoder::rewind()
{
	BSTONE_ASSERT(is_initialized());
	OplUtility::initialize_registers(*emulator_);
	reader_.set_position(2);
	command_index_ = 0;
	remains_count_ = 0;
	samples_per_tick_ = 0;
	return true;
}

int OplMusicDecoder::get_dst_length_in_samples() const
{
	BSTONE_ASSERT(is_initialized());
	return dst_length_in_samples_;
}

int OplMusicDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return impl_get_channel_count();
}

int OplMusicDecoder::decode(int dst_count, float* dst_samples)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(dst_count >= 0);
	if (command_index_ == commands_count_ && remains_count_ == 0)
		return 0;
	int decoded_samples_count = 0;
	int dst_data_index = 0;
	int dst_remain_count = dst_count;
	for (bool quit = false; !quit; )
	{
		if (remains_count_ > 0)
		{
			const int count = std::min(dst_remain_count, remains_count_);
			emulator_->generate(count, dst_samples + dst_data_index * impl_get_channel_count());
			dst_data_index += count;
			dst_remain_count -= count;
			remains_count_ -= count;
			decoded_samples_count += count;
		}
		else
		{
			int delay = 0;
			while (command_index_ < commands_count_ && delay == 0)
			{
				const int command_reg = reader_.read_u8();
				const int command_value = reader_.read_u8();
				delay = reader_.read_u16_le();
				emulator_->write_buffered(command_reg, command_value);
				++command_index_;
			}
			if (delay > 0)
			{
				samples_per_tick_ += delay * emulator_->get_sample_rate();
				remains_count_ = samples_per_tick_ / tick_rate;
				samples_per_tick_ %= tick_rate;
			}
		}
		quit = (command_index_ == commands_count_ && remains_count_ == 0) || dst_remain_count == 0;
	}
	return decoded_samples_count;
}

void OplMusicDecoder::impl_uninitialize()
{
	is_initialized_ = false;
}

int OplMusicDecoder::impl_get_channel_count() const
{
	return emulator_->get_channel_count();
}

} // namespace

// =====================================

AudioDecoderUPtr make_opl_music_audio_decoder(OplEmulatorType opl3_type)
{
	return std::make_unique<OplMusicDecoder>(opl3_type);
}

} // namespace bstone
