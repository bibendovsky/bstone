/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Nuked OPL3 wrapper


#include "bstone_nuked_opl3.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_opl_emulator.h"
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
	int get_channel_count() const override;

	void write(int fm_port, int fm_value) override;
	void write_buffered(int fm_port, int fm_value) override;

	void generate(int count, float* buffer) override;

	void reset() override;

	int get_min_sample_rate() const override;

private:
	inline static constexpr int channel_count = 2;

	bool is_initialized_{};
	int sample_rate_{};
	opl3_chip emulator_{};

	static int get_max_samples_count();

	void generate_block(float* sample_buffer, int sample_count);
};

// -------------------------------------

OplEmulatorType NukedOpl3::get_type() const
{
	return OplEmulatorType::nuked_opl3;
}

bool NukedOpl3::initialize(const OplEmulatorInitParam& param)
{
	terminate();
	sample_rate_ = std::max(param.sample_rate, get_min_sample_rate());
	OPL3_Reset(&emulator_, static_cast<std::uint32_t>(sample_rate_));
	is_initialized_ = true;
	return true;
}

void NukedOpl3::terminate()
{
	is_initialized_ = false;
	sample_rate_ = 0;
	emulator_ = opl3_chip{};
}

bool NukedOpl3::is_initialized() const
{
	return is_initialized_;
}

int NukedOpl3::get_sample_rate() const
{
	return sample_rate_;
}

int NukedOpl3::get_channel_count() const
{
	return 2;
}

void NukedOpl3::write(int fm_port, int fm_value)
{
	if (!is_initialized_)
		return;
	OPL3_WriteReg(&emulator_, static_cast<std::uint16_t>(fm_port), static_cast<std::uint8_t>(fm_value));
}

void NukedOpl3::write_buffered(int fm_port, int fm_value)
{
	if (is_initialized_)
		OPL3_WriteRegBuffered(&emulator_, static_cast<std::uint16_t>(fm_port), static_cast<std::uint8_t>(fm_value));
}

void NukedOpl3::generate(int count, float* buffer)
{
	if (!is_initialized_)
		return;
	if (count < 1)
		return;
	if (buffer == nullptr)
		return;
	for (int remain_count = count; remain_count > 0; )
	{
		const int generate_count = std::min(remain_count, get_max_samples_count());
		generate_block(buffer, generate_count);
		remain_count -= generate_count;
		buffer += generate_count * channel_count;
	}
}

void NukedOpl3::reset()
{
	if (is_initialized_)
		initialize(OplEmulatorInitParam{.sample_rate = sample_rate_});
}

int NukedOpl3::get_min_sample_rate() const
{
	return 8'000;
}

int NukedOpl3::get_max_samples_count()
{
	return OPL_WRITEBUF_SIZE;
}

void NukedOpl3::generate_block(float* sample_buffer, int sample_count)
{
	std::int16_t opl3_samples[4];
	for (int i = 0; i < sample_count; ++i)
	{
		OPL3_Generate4ChResampled(&emulator_, opl3_samples);
		sample_buffer[i * 2 + 0] = AudioSampleConverter::s16_to_f32(opl3_samples[0]);
		sample_buffer[i * 2 + 1] = AudioSampleConverter::s16_to_f32(opl3_samples[1]);
	}
}

} // namespace

// =====================================

OplEmulatorUPtr make_nuked_opl3()
{
	return std::make_unique<NukedOpl3>();
}

} // namespace bstone
