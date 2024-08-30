/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

//
// DOSBox DBOPL wrapper.
//

#include "bstone_dosbox_dbopl.h"
#include <algorithm>
#include <type_traits>
#include <vector>
#include "dbopl.h"
#include "bstone_audio_sample_converter.h"


namespace bstone
{

namespace
{

//
// DOSBox DBOPL wrapper.
//
class DosboxDbopl final : public Opl3
{
public:
	DosboxDbopl();
	~DosboxDbopl() override;

	Opl3Type get_type() const noexcept override;

	void initialize(int sample_rate) override;
	void uninitialize() override;

	bool is_initialized() const noexcept override;
	int get_sample_rate() const noexcept override;

	void write(int fm_port, int fm_value) override;
	bool generate(int count, std::int16_t* buffer) override;
	bool generate(const int count, float* buffer) override;

	bool reset() override;

	int get_min_sample_rate() const noexcept override;

private:
	struct S16Tag{};
	struct F32Tag{};

	using Buffer = std::vector<std::int16_t>;

	bool is_initialized_{};
	int sample_rate_{};
	DBOPL::Handler emulator_{};
	MixerChannel channel_{};
	Buffer samples_{};

	// Returns a maximum number of output samples generated at once.
	// (Emulator dependent value)
	static int get_max_samples_count() noexcept;

	void generate_block(int count, std::int16_t* buffer, S16Tag);
	void generate_block(int count, float* buffer, F32Tag);

	template<typename T>
	void generate_block(int count, T* buffer);

	template<typename T>
	bool generate(int count, T* buffer);
}; // DosboxDbopl

// --------------------------------------------------------------------------

DosboxDbopl::DosboxDbopl() = default;
DosboxDbopl::~DosboxDbopl() = default;

Opl3Type DosboxDbopl::get_type() const noexcept
{
	return Opl3Type::dbopl;
}

void DosboxDbopl::initialize(int sample_rate)
{
	uninitialize();
	sample_rate_ = std::max(sample_rate, get_min_sample_rate());
	channel_ = {};
	samples_.resize(get_max_samples_count());
	emulator_ = {};
	emulator_.Init(sample_rate_);
	is_initialized_ = true;
}

void DosboxDbopl::uninitialize()
{
	is_initialized_ = false;
	sample_rate_ = 0;
	emulator_ = {};
	channel_ = {};
}

bool DosboxDbopl::is_initialized() const noexcept
{
	return is_initialized_;
}

int DosboxDbopl::get_sample_rate() const noexcept
{
	return sample_rate_;
}

void DosboxDbopl::write(int fm_port, int fm_value)
{
	if (!is_initialized_)
	{
		return;
	}

	emulator_.WriteReg(static_cast<Bit32u>(fm_port), static_cast<Bit8u>(fm_value));
}

bool DosboxDbopl::generate(int count, std::int16_t* buffer)
{
	return generate<std::int16_t>(count, buffer);
}

bool DosboxDbopl::generate(int count, float* buffer)
{
	return generate<float>(count, buffer);
}

bool DosboxDbopl::reset()
{
	if (!is_initialized_)
	{
		return false;
	}

	initialize(sample_rate_);
	return true;
}

int DosboxDbopl::get_min_sample_rate() const noexcept
{
	return 8'000;
}

int DosboxDbopl::get_max_samples_count() noexcept
{
	return 512;
}

void DosboxDbopl::generate_block(int count, std::int16_t* buffer, S16Tag)
{
	channel_.set_buffer(buffer);
	emulator_.Generate(&channel_, static_cast<Bitu>(count));
}

void DosboxDbopl::generate_block(int count, float* buffer, F32Tag)
{
	channel_.set_buffer(samples_.data());
	emulator_.Generate(&channel_, static_cast<Bitu>(count));
	std::transform(samples_.cbegin(), samples_.cbegin() + count, buffer, AudioSampleConverter::s16_to_f32);
}

template<typename T>
void DosboxDbopl::generate_block(int count, T* buffer)
{
	using Tag = std::conditional_t<
		std::is_same<T, std::int16_t>::value,
		S16Tag,
		std::conditional_t<
			std::is_same<T, float>::value,
			F32Tag,
			void
		>
	>;

	generate_block(count, buffer, Tag{});
}

template<typename T>
bool DosboxDbopl::generate(int count, T* buffer)
{
	if (!is_initialized_)
	{
		return false;
	}

	if (count < 1)
	{
		return false;
	}

	if (!buffer)
	{
		return false;
	}

	auto remain_count = count;

	while (remain_count > 0)
	{
		const auto generate_count = std::min(remain_count, get_max_samples_count());
		generate_block<T>(generate_count, buffer);
		remain_count -= generate_count;
		buffer += generate_count;
	}

	return true;
}

} // namespace

// ==========================================================================

Opl3UPtr make_dbopl_opl3()
{
	return std::make_unique<DosboxDbopl>();
}

} // bstone
