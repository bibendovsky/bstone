/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// OPL SFX decoder

#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_opl_emulator.h"
#include "bstone_opl_utility.h"
#include <algorithm>

namespace bstone {

namespace {

class OplSfxDecoder final : public AudioDecoder
{
public:
	explicit OplSfxDecoder(OplEmulatorType opl_emulator_type);
	~OplSfxDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void uninitialize() override;
	bool is_initialized() const override;
	int decode(int frame_count, float* samples) override;
	bool rewind() override;
	int get_dst_length_in_samples() const override;
	int get_channel_count() const override;

private:
	inline static constexpr int header_size = 23; // Original size of AdLibSound structure.
	inline static constexpr int tick_rate = 140;

	OplEmulatorUPtr emulator_{};

	bool is_initialized_{};

	MemoryBinaryReader reader_{};
	OplInstrument instrument_{};
	int commands_count_{};
	int command_offset_{};
	int samples_per_tick_{};
	int frames_left_{};
	int hf_{};
	int dst_length_in_samples_{};

	void impl_uninitialize();
	int impl_get_channel_count() const;
};

// -------------------------------------

OplSfxDecoder::OplSfxDecoder(OplEmulatorType opl_emulator_type)
	:
	emulator_{make_opl_emulator(opl_emulator_type)}
{}

bool OplSfxDecoder::initialize(const AudioDecoderInitParam& param)
{
	impl_uninitialize();
	if (emulator_ == nullptr)
		return false;
	if (param.src_raw_data == nullptr)
		return false;
	if (param.src_raw_size < 0)
		return false;
	if (param.dst_rate < 1)
		return false;
	emulator_->initialize(OplEmulatorInitParam{.sample_rate = param.dst_rate});
	OplUtility::initialize_registers(*emulator_);
	reader_ = MemoryBinaryReader{param.src_raw_data, param.src_raw_size};
	if (!reader_.can_read_x32())
		return false;
	const int sfx_length = reader_.read_s32_le();
	if (sfx_length <= 0)
		return false;
	if (!reader_.can_read_n(header_size + sfx_length))
		return false;
	reader_.skip(2); // Priority.
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
	reader_.skip(6); // nConn, voice, mode and 3 unused octets
	if (instrument_.m_sus == 0 && instrument_.c_sus == 0)
		return false;
	hf_ = reader_.read_u8();
	hf_ = ((hf_ & 7) << 2) | 0x20;
	OplUtility::initialize_registers(*emulator_);
	OplUtility::set_instrument(*emulator_, instrument_);
	command_offset_ = 0;
	commands_count_ = sfx_length;
	samples_per_tick_ = 0;
	dst_length_in_samples_ = commands_count_ * emulator_->get_sample_rate() / tick_rate;
	frames_left_ = 0;
	is_initialized_ = true;
	return true;
}

void OplSfxDecoder::uninitialize()
{
	impl_uninitialize();
}

bool OplSfxDecoder::rewind()
{
	BSTONE_ASSERT(is_initialized());
	OplUtility::initialize_registers(*emulator_);
	OplUtility::set_instrument(*emulator_, instrument_);
	command_offset_ = 0;
	frames_left_ = 0;
	samples_per_tick_ = 0;
	reader_.set_position(header_size);
	return true;
}

int OplSfxDecoder::get_dst_length_in_samples() const
{
	BSTONE_ASSERT(is_initialized());
	return dst_length_in_samples_;
}

int OplSfxDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return impl_get_channel_count();
}

bool OplSfxDecoder::is_initialized() const
{
	return is_initialized_;
}

int OplSfxDecoder::decode(int frame_count, float* samples)
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
			if (command_offset_ < commands_count_)
			{
				const int lf = reader_.read_u8();
				if (lf > 0)
				{
					emulator_->write_deferred(OplUtility::al_freq_l, lf);
					emulator_->write_deferred(OplUtility::al_freq_h, hf_);
				}
				else
					emulator_->write_deferred(OplUtility::al_freq_h, 0x00);
				++command_offset_;
				samples_per_tick_ += emulator_->get_sample_rate();
				frames_left_ = samples_per_tick_ / tick_rate;
				samples_per_tick_ %= tick_rate;
			}
		}
		quit = (command_offset_ == commands_count_ && frames_left_ == 0) || frames_left == 0;
	}
	return decoded_frame_count;
}

void OplSfxDecoder::impl_uninitialize()
{
	is_initialized_ = false;
}

int OplSfxDecoder::impl_get_channel_count() const
{
	return emulator_->get_channel_count();
}

} // namespace

// =====================================

AudioDecoderUPtr make_opl_sfx_audio_decoder(OplEmulatorType opl_emulator_type)
{
	return std::make_unique<OplSfxDecoder>(opl_emulator_type);
}

} // namespace bstone
