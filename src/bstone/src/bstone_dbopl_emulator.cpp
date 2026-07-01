/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// DOSBox DBOPL emulator

#include "bstone_dbopl_emulator.h"
#include "bstone_assert.h"
#include "bstone_audio_sample_converter.h"
#include <algorithm>
#include <vector>
#include <cstdint>
#include "dbopl.h"

namespace bstone {

namespace {

class DboplEmulator final : public OplEmulator
{
public:
	DboplEmulator() = default;
	~DboplEmulator() override = default;

	OplEmulatorType get_type() const override;
	bool initialize(const OplEmulatorInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	int get_sample_rate() const override;
	int get_min_sample_rate() const override;
	int get_channel_count() const override;
	void write_immediate(int address, int value) override;
	void write_deferred(int address, int value) override;
	void generate_frames(float* samples, int frame_count) override;

private:
	inline static constexpr int channel_count = 1;
	inline static constexpr int min_sample_rate = 11025;
	inline static constexpr int max_frames_per_block = 512;
	inline static constexpr int max_samples_per_block = max_frames_per_block * channel_count;

	using SamplesS16 = std::vector<std::int16_t>;

	bool is_initialized_{};
	int sample_rate_{};
	DBOPL::Handler emulator_{};
	MixerChannel mixer_channel_{};
	SamplesS16 samples_s16_{};

	void generate_frames_block(float* samples, int frame_count);
};

// -------------------------------------

OplEmulatorType DboplEmulator::get_type() const
{
	return OplEmulatorType::dbopl;
}

bool DboplEmulator::initialize(const OplEmulatorInitParam& param)
{
	terminate();
	if (param.sample_rate < min_sample_rate)
		return false;
	sample_rate_ = param.sample_rate;
	samples_s16_.resize(max_samples_per_block);
	emulator_.Init(sample_rate_);
	is_initialized_ = true;
	return true;
}

void DboplEmulator::terminate()
{
	is_initialized_ = false;
}

bool DboplEmulator::is_initialized() const
{
	return is_initialized_;
}

int DboplEmulator::get_sample_rate() const
{
	BSTONE_ASSERT(is_initialized());
	return sample_rate_;
}

int DboplEmulator::get_min_sample_rate() const
{
	BSTONE_ASSERT(is_initialized());
	return min_sample_rate;
}

int DboplEmulator::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

void DboplEmulator::write_immediate(int address, int value)
{
	BSTONE_ASSERT(is_initialized());
	emulator_.WriteReg(static_cast<Bit32u>(address), static_cast<Bit8u>(value));
}

void DboplEmulator::write_deferred(int address, int value)
{
	BSTONE_ASSERT(is_initialized());
	emulator_.WriteReg(static_cast<Bit32u>(address), static_cast<Bit8u>(value));
}

void DboplEmulator::generate_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(frame_count >= 0);
	for (int frame_offset = 0; frame_offset < frame_count; )
	{
		const int block_frame_count = std::min(frame_count - frame_offset, max_frames_per_block);
		generate_frames_block(&samples[frame_offset], block_frame_count);
		frame_offset += block_frame_count;
	}
}

void DboplEmulator::generate_frames_block(float* samples, int frame_count)
{
	BSTONE_ASSERT(frame_count >= 0);
	mixer_channel_.set_buffer(samples_s16_.data());
	emulator_.Generate(&mixer_channel_, static_cast<Bitu>(frame_count));
	const std::int16_t* const src_samples = samples_s16_.data();
	for (int frame_offset = 0; frame_offset < frame_count; ++frame_offset)
		samples[frame_offset] = AudioSampleConverter::s16_to_f32(src_samples[frame_offset]);
}

} // namespace

// =====================================

OplEmulatorUPtr make_dbopl_emulator()
{
	return std::make_unique<DboplEmulator>();
}

} // namespace bstone
