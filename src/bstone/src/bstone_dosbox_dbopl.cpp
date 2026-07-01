/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// DOSBox DBOPL wrapper

#include "bstone_dosbox_dbopl.h"
#include "bstone_audio_sample_converter.h"
#include <algorithm>
#include <type_traits>
#include <vector>
#include "dbopl.h"

namespace bstone {

namespace {

class DosboxDbopl final : public OplEmulator
{
public:
	DosboxDbopl() = default;
	~DosboxDbopl() override = default;

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
	inline static constexpr int channel_count = 1;

	using SamplesS16 = std::vector<std::int16_t>;

	bool is_initialized_{};
	int sample_rate_{};
	DBOPL::Handler emulator_{};
	MixerChannel channel_{};
	SamplesS16 samples_s16_{};

	// Returns a maximum number of output samples generated at once.
	// (Emulator dependent value)
	static int get_max_samples_count();

	void generate_block(float* sample_buffer, int sample_count);
};

// -------------------------------------

OplEmulatorType DosboxDbopl::get_type() const
{
	return OplEmulatorType::dbopl;
}

bool DosboxDbopl::initialize(const OplEmulatorInitParam& param)
{
	terminate();
	sample_rate_ = std::max(param.sample_rate, get_min_sample_rate());
	channel_ = {};
	samples_s16_.resize(get_max_samples_count());
	emulator_ = {};
	emulator_.Init(sample_rate_);
	is_initialized_ = true;
	return true;
}

void DosboxDbopl::terminate()
{
	is_initialized_ = false;
	sample_rate_ = 0;
	emulator_ = DBOPL::Handler{};
	channel_ = MixerChannel{};
}

bool DosboxDbopl::is_initialized() const
{
	return is_initialized_;
}

int DosboxDbopl::get_sample_rate() const
{
	return sample_rate_;
}

int DosboxDbopl::get_channel_count() const
{
	return channel_count;
}

void DosboxDbopl::write(int fm_port, int fm_value)
{
	if (!is_initialized_)
		return;
	emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

void DosboxDbopl::write_buffered(int fm_port, int fm_value)
{
	if (is_initialized_)
		emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

void DosboxDbopl::generate(int count, float* buffer)
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

void DosboxDbopl::reset()
{
	if (is_initialized_)
		initialize(OplEmulatorInitParam{.sample_rate = sample_rate_});
}

int DosboxDbopl::get_min_sample_rate() const
{
	return 8'000;
}

int DosboxDbopl::get_max_samples_count()
{
	return 512;
}

void DosboxDbopl::generate_block(float* sample_buffer, int sample_count)
{
	channel_.set_buffer(samples_s16_.data());
	emulator_.Generate(&channel_, static_cast<Bitu>(sample_count));
	std::transform(samples_s16_.cbegin(), samples_s16_.cbegin() + sample_count, sample_buffer, AudioSampleConverter::s16_to_f32);
}

} // namespace

// =====================================

OplEmulatorUPtr make_dbopl_opl3()
{
	return std::make_unique<DosboxDbopl>();
}

} // namespace bstone
