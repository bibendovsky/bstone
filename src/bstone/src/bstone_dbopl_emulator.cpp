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
	int get_channel_count() const override;
	void write(int fm_port, int fm_value) override;
	void write_buffered(int fm_port, int fm_value) override;
	void generate(int count, float* buffer) override;
	int get_min_sample_rate() const override;

private:
	inline static constexpr int channel_count = 1;
	inline static constexpr int max_frame_count = 512;
	inline static constexpr int max_sample_count = max_frame_count * channel_count;

	using SamplesS16 = std::vector<std::int16_t>;

	bool is_initialized_{};
	int sample_rate_{};
	DBOPL::Handler emulator_{};
	MixerChannel mixer_channel_{};
	SamplesS16 samples_s16_{};

	void generate_block(float* dst_samples, int sample_count);
};

// -------------------------------------

OplEmulatorType DboplEmulator::get_type() const
{
	return OplEmulatorType::dbopl;
}

bool DboplEmulator::initialize(const OplEmulatorInitParam& param)
{
	terminate();
	sample_rate_ = std::max(param.sample_rate, get_min_sample_rate());
	samples_s16_.resize(max_sample_count);
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

int DboplEmulator::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

void DboplEmulator::write(int fm_port, int fm_value)
{
	BSTONE_ASSERT(is_initialized());
	emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

void DboplEmulator::write_buffered(int fm_port, int fm_value)
{
	BSTONE_ASSERT(is_initialized());
	emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

void DboplEmulator::generate(int count, float* buffer)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(count >= 0);
	for (int remain_count = count; remain_count > 0; )
	{
		const int generate_count = std::min(remain_count, max_sample_count);
		generate_block(buffer, generate_count);
		remain_count -= generate_count;
		buffer += generate_count * channel_count;
	}
}

int DboplEmulator::get_min_sample_rate() const
{
	return 11025;
}

void DboplEmulator::generate_block(float* dst_samples, int sample_count)
{
	mixer_channel_.set_buffer(samples_s16_.data());
	emulator_.Generate(&mixer_channel_, static_cast<Bitu>(sample_count));
	const std::int16_t* const src_samples = samples_s16_.data();
	for (int i = 0; i < sample_count; ++i)
		dst_samples[i] = AudioSampleConverter::s16_to_f32(src_samples[i]);
}

} // namespace

// =====================================

OplEmulatorUPtr make_dbopl_emulator()
{
	return std::make_unique<DboplEmulator>();
}

} // namespace bstone
