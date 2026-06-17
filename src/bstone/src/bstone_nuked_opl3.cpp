/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Nuked OPL3 wrapper


#include "bstone_nuked_opl3.h"
#include "opl3.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_opl3.h"
#include <algorithm>

namespace bstone {

namespace {

class NukedOpl3 final : public Opl3
{
public:
	~NukedOpl3() override = default;

	Opl3Type get_type() const override;

	void initialize(int sample_rate) override;
	void uninitialize() override;

	bool is_initialized() const override;
	int get_sample_rate() const override;

	void write(int fm_port, int fm_value) override;
	void write_buffered(int fm_port, int fm_value) override;

	bool generate(int count, float* buffer) override;

	bool reset() override;

	int get_min_sample_rate() const override;

private:
	bool is_initialized_{};
	int sample_rate_{};
	opl3_chip emulator_{};

	static int get_max_samples_count();

	void generate_block(int count, float* buffer);
};

// -------------------------------------

Opl3Type NukedOpl3::get_type() const
{
	return Opl3Type::nuked;
}

void NukedOpl3::initialize(int sample_rate)
{
	uninitialize();
	sample_rate_ = std::max(sample_rate, get_min_sample_rate());
	OPL3_Reset(&emulator_, static_cast<std::uint32_t>(sample_rate_));
	is_initialized_ = true;
}

void NukedOpl3::uninitialize()
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

bool NukedOpl3::generate(int count, float* buffer)
{
	if (!is_initialized_)
		return false;
	if (count < 1)
		return false;
	if (buffer == nullptr)
		return false;
	int remain_count = count;
	while (remain_count > 0)
	{
		const int generate_count = std::min(remain_count, get_max_samples_count());
		generate_block(generate_count, buffer);
		remain_count -= generate_count;
		buffer += generate_count;
	}
	return true;
}

bool NukedOpl3::reset()
{
	if (!is_initialized_)
		return false;
	initialize(sample_rate_);
	return true;
}

int NukedOpl3::get_min_sample_rate() const
{
	return 8'000;
}

int NukedOpl3::get_max_samples_count()
{
	return OPL_WRITEBUF_SIZE;
}

void NukedOpl3::generate_block(int count, float* buffer)
{
	std::int16_t opl3_samples[4];
	for (int i = 0; i < count; ++i)
	{
		OPL3_Generate4ChResampled(&emulator_, opl3_samples);
		buffer[i] = AudioSampleConverter::s16_to_f32(static_cast<std::int16_t>((opl3_samples[0] + opl3_samples[1]) / 2));
	}
}

} // namespace

// =====================================

Opl3UPtr make_nuked_opl3()
{
	return std::make_unique<NukedOpl3>();
}

} // namespace bstone
