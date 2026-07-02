/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2021-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Nuked OPL3 emulator

#include "bstone_nuked_opl3_emulator.h"
#include "bstone_assert.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_opl_emulator.h"
#include <cstdint>
#include <algorithm>
#include "opl3.h"

namespace bstone {

namespace {

class NukedOpl3 final : public OplEmulator
{
public:
	~NukedOpl3() override = default;

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
	inline static constexpr int channel_count = 2;
	inline static constexpr int min_sample_rate = 11025;
	inline static constexpr int max_frames_per_block = OPL_WRITEBUF_SIZE;
	inline static constexpr int max_samples_per_block = max_frames_per_block * channel_count;

	bool is_initialized_{};
	int sample_rate_{};
	opl3_chip emulator_{};

	void generate_frames_block(float* samples, int frame_count);
};

// -------------------------------------

OplEmulatorType NukedOpl3::get_type() const
{
	return OplEmulatorType::nuked_opl3;
}

bool NukedOpl3::initialize(const OplEmulatorInitParam& param)
{
	terminate();
	if (param.sample_rate < min_sample_rate)
		return false;
	sample_rate_ = param.sample_rate;
	OPL3_Reset(&emulator_, static_cast<std::uint32_t>(sample_rate_));
	is_initialized_ = true;
	return true;
}

void NukedOpl3::terminate()
{
	is_initialized_ = false;
}

bool NukedOpl3::is_initialized() const
{
	return is_initialized_;
}

int NukedOpl3::get_sample_rate() const
{
	BSTONE_ASSERT(is_initialized());
	return sample_rate_;
}

int NukedOpl3::get_min_sample_rate() const
{
	BSTONE_ASSERT(is_initialized());
	return min_sample_rate;
}

int NukedOpl3::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

void NukedOpl3::write_immediate(int address, int value)
{
	BSTONE_ASSERT(is_initialized());
	OPL3_WriteReg(&emulator_, static_cast<std::uint16_t>(address), static_cast<std::uint8_t>(value));
}

void NukedOpl3::write_deferred(int address, int value)
{
	BSTONE_ASSERT(is_initialized());
	OPL3_WriteRegBuffered(&emulator_, static_cast<std::uint16_t>(address), static_cast<std::uint8_t>(value));
}

void NukedOpl3::generate_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
	BSTONE_ASSERT(frame_count >= 0);
	for (int frame_offset = 0; frame_offset < frame_count; )
	{
		const int block_frame_count = std::min(frame_count - frame_offset, max_frames_per_block);
		generate_frames_block(&samples[frame_offset * channel_count], block_frame_count);
		frame_offset += block_frame_count;
	}
}

void NukedOpl3::generate_frames_block(float* samples, int frame_count)
{
	BSTONE_ASSERT(frame_count >= 0);
	std::int16_t quad_samples[4];
	for (int frame_offset = 0; frame_offset < frame_count; ++frame_offset)
	{
		OPL3_Generate4ChResampled(&emulator_, quad_samples);
		for (int i_channel = 0; i_channel < channel_count; ++i_channel)
			samples[frame_offset * channel_count + i_channel] = AudioSampleConverter::s16_to_f32(quad_samples[i_channel]);
	}
}

} // namespace

// =====================================

OplEmulatorUPtr make_nuked_opl3_emulator()
{
	return std::make_unique<NukedOpl3>();
}

} // namespace bstone
