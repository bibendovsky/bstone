/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// AdLib sound effects decoder

#include "bstone_adlib_decoder.h"
#include "bstone_audio_decoder.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_opl3.h"
#include <algorithm>

namespace bstone {

namespace {

class AdlibSfxDecoder final : public AudioDecoder
{
public:
	AdlibSfxDecoder(Opl3Type opl3_type);
	~AdlibSfxDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void uninitialize() override;
	bool is_initialized() const override;

	int decode(int dst_count, float* dst_data) override;
	bool rewind() override;

	int get_dst_length_in_samples() const override;
	int get_channel_count() const override;

	// Returns a number of calls per second of
	// original interrupt routine.
	static int get_tick_rate();

private:
	Opl3UPtr emulator_{};

	bool is_initialized_{};

	MemoryBinaryReader reader_{};
	adlib::Instrument instrument_{};
	int commands_count_{};
	int command_index_{};
	int samples_per_tick_{};
	int remains_count_{};
	int hf_{};
	int dst_length_in_samples_{};

	void uninitialize_internal();
	int impl_get_channel_count() const;

	// Returns an original size of an AdLibSound structure.
	static int get_header_size();
};

// -------------------------------------

AdlibSfxDecoder::AdlibSfxDecoder(Opl3Type opl3_type)
	:
	emulator_{make_opl3(opl3_type)}
{}

bool AdlibSfxDecoder::initialize(const AudioDecoderInitParam& param)
{
	uninitialize_internal();
	if (emulator_ == nullptr)
		return false;
	if (param.src_raw_data == nullptr)
		return false;
	if (param.src_raw_size < 0)
		return false;
	if (param.dst_rate < 1)
		return false;
	emulator_->initialize(Opl3InitParam{.sample_rate = param.dst_rate});
	adlib::initialize_registers(emulator_.get());
	reader_ = MemoryBinaryReader{param.src_raw_data, param.src_raw_size};
	if (!reader_.can_read_x32())
		return false;
	const int sfx_length = reader_.read_s32_le();
	if (sfx_length <= 0)
		return false;
	if (!reader_.can_read_n(get_header_size() + sfx_length))
		return false;
	// Skip priority.
	reader_.skip(2);
	instrument_.m_char = reader_.read_u8();
	instrument_.c_char = reader_.read_u8();
	instrument_.m_scale = reader_.read_u8();
	instrument_.c_scale = reader_.read_u8();
	instrument_.m_attack = reader_.read_u8();
	instrument_.c_attack = reader_.read_u8();
	instrument_.m_sus = reader_.read_u8();
	instrument_.c_sus = reader_.read_u8();
	instrument_.m_wave = reader_.read_u8();
	instrument_.c_wave = reader_.read_u8();
	// Skip nConn, voice, mode and 3 unused octets
	reader_.skip(6);
	if (instrument_.m_sus == 0 && instrument_.c_sus == 0)
		return false;
	hf_ = reader_.read_u8();
	hf_ = ((hf_ & 7) << 2) | 0x20;
	adlib::set_instrument(emulator_.get(), instrument_);
	command_index_ = 0;
	commands_count_ = sfx_length;
	samples_per_tick_ = 0;
	dst_length_in_samples_ = commands_count_ * emulator_->get_sample_rate() / get_tick_rate();
	remains_count_ = 0;
	is_initialized_ = true;
	return true;
}

void AdlibSfxDecoder::uninitialize()
{
	uninitialize_internal();
}

bool AdlibSfxDecoder::rewind()
{
	if (!emulator_->reset())
		return false;
	adlib::initialize_registers(emulator_.get());
	adlib::set_instrument(emulator_.get(), instrument_);
	command_index_ = 0;
	remains_count_ = 0;
	samples_per_tick_ = 0;
	reader_.set_position(get_header_size());
	return true;
}

int AdlibSfxDecoder::get_dst_length_in_samples() const
{
	return dst_length_in_samples_;
}

int AdlibSfxDecoder::get_channel_count() const
{
	return impl_get_channel_count();
}

bool AdlibSfxDecoder::is_initialized() const
{
	return is_initialized_;
}

int AdlibSfxDecoder::decode(int dst_count, float* dst_data)
{
	if (!is_initialized_)
		return 0;
	if (dst_count < 1)
		return 0;
	if (dst_data == nullptr)
		return 0;
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
			emulator_->generate(count, &dst_data[dst_data_index * impl_get_channel_count()]);
			dst_data_index += count;
			dst_remain_count -= count;
			remains_count_ -= count;
			decoded_samples_count += count;
		}
		else
		{
			if (command_index_ < commands_count_)
			{
				const int lf = reader_.read_u8();
				if (lf > 0)
				{
					emulator_->write_buffered(adlib::al_freq_l, lf);
					emulator_->write_buffered(adlib::al_freq_h, hf_);
				}
				else
					emulator_->write_buffered(adlib::al_freq_h, 0x00);
				++command_index_;
				const int tick_rate = get_tick_rate();
				samples_per_tick_ += emulator_->get_sample_rate();
				remains_count_ = samples_per_tick_ / tick_rate;
				samples_per_tick_ %= tick_rate;
			}
		}
		quit = ((command_index_ == commands_count_ && remains_count_ == 0) || dst_remain_count == 0);
	}
	return decoded_samples_count;
}

void AdlibSfxDecoder::uninitialize_internal()
{
	reader_ = MemoryBinaryReader{};
	instrument_ = adlib::Instrument{};
	commands_count_ = 0;
	command_index_ = 0;
	samples_per_tick_ = 0;
	remains_count_ = 0;
	hf_ = 0;
	dst_length_in_samples_ = 0;
}

int AdlibSfxDecoder::impl_get_channel_count() const
{
	return emulator_->get_channel_count();
}

int AdlibSfxDecoder::get_tick_rate()
{
	return 140;
}

int AdlibSfxDecoder::get_header_size()
{
	return 23;
}

} // namespace

// =====================================

AudioDecoderUPtr make_adlib_sfx_audio_decoder(Opl3Type opl3_type)
{
	return std::make_unique<AdlibSfxDecoder>(opl3_type);
}

} // namespace bstone
