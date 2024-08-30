/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// Nuked OPL3 wrapper.
//

#include <algorithm>
#include "bstone_nuked_opl3.h"
#include <type_traits>
#include <vector>
#include "opl3.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_opl3.h"

namespace bstone
{

namespace
{

//
// Nuked OPL3 wrapper.
//
class NukedOpl3 final : public Opl3
{
public:
	~NukedOpl3() override;

	Opl3Type get_type() const noexcept override;

	void initialize(int sample_rate) override;
	void uninitialize() override;

	bool is_initialized() const noexcept override;
	int get_sample_rate() const noexcept override;

	void write(int fm_port, int fm_value) override;

	bool generate(int count, std::int16_t* buffer) override;
	bool generate(int count, float* buffer) override;

	bool reset() override;

	int get_min_sample_rate() const noexcept override;

private:
	struct S16Tag{};
	struct F32Tag{};

	using Buffer = std::vector<std::int16_t>;

	bool is_initialized_{};
	int sample_rate_{};
	opl3_chip emulator_{};
	Buffer samples_{};

	static int get_max_samples_count() noexcept;

	void generate_block(int count, std::int16_t* buffer, S16Tag);
	void generate_block(int count, float* buffer, F32Tag);

	template<typename T>
	void generate_block(int count, T* buffer);

	template<typename T>
	bool generate(int count, T* buffer);
}; // NukedOpl3

// --------------------------------------------------------------------------

NukedOpl3::~NukedOpl3() = default;

Opl3Type NukedOpl3::get_type() const noexcept
{
	return Opl3Type::nuked;
}

void NukedOpl3::initialize(int sample_rate)
{
	uninitialize();

	sample_rate_ = std::max(sample_rate, get_min_sample_rate());
	// The emulator outputs stereo samples.
	const auto buffer_size = get_max_samples_count() * 2;
	samples_.resize(buffer_size);
	OPL3_Reset(&emulator_, static_cast<std::uint32_t>(sample_rate_));
	is_initialized_ = true;
}

void NukedOpl3::uninitialize()
{
	is_initialized_ = false;
	sample_rate_ = 0;
	emulator_ = {};
}

bool NukedOpl3::is_initialized() const noexcept
{
	return is_initialized_;
}

int NukedOpl3::get_sample_rate() const noexcept
{
	return sample_rate_;
}

void NukedOpl3::write(int fm_port, int fm_value)
{
	if (!is_initialized_)
	{
		return;
	}

	OPL3_WriteReg(&emulator_, static_cast<std::uint16_t>(fm_port), static_cast<std::uint8_t>(fm_value));
}

bool NukedOpl3::generate(int count, std::int16_t* buffer)
{
	return generate<std::int16_t>(count, buffer);
}

bool NukedOpl3::generate(int count, float* buffer)
{
	return generate<float>(count, buffer);
}

bool NukedOpl3::reset()
{
	if (!is_initialized_)
	{
		return false;
	}

	initialize(sample_rate_);
	return true;
}

int NukedOpl3::get_min_sample_rate() const noexcept
{
	return 8'000;
}

int NukedOpl3::get_max_samples_count() noexcept
{
	return OPL_WRITEBUF_SIZE;
}

void NukedOpl3::generate_block(int count, std::int16_t* buffer, S16Tag)
{
	OPL3_GenerateStream(&emulator_, samples_.data(), static_cast<std::uint32_t>(count));

	auto src_samples = samples_.data();

	for (auto i = 0; i < count; ++i)
	{
		const auto src_left_sample = static_cast<int>(*src_samples++);
		const auto src_right_sample = static_cast<int>(*src_samples++);
		const auto src_sample = static_cast<std::int16_t>((src_left_sample + src_right_sample) / 2);
		buffer[i] = src_sample;
	}
}

void NukedOpl3::generate_block(int count, float* buffer, F32Tag)
{
	OPL3_GenerateStream(&emulator_, samples_.data(), static_cast<std::uint32_t>(count));

	auto src_samples = samples_.data();

	for (auto i = 0; i < count; ++i)
	{
		const auto src_left_sample = static_cast<int>(*src_samples++);
		const auto src_right_sample = static_cast<int>(*src_samples++);
		const auto src_sample = static_cast<std::int16_t>((src_left_sample + src_right_sample) / 2);
		buffer[i] = AudioSampleConverter::s16_to_f32(src_sample);
	}
}

template<typename T>
void NukedOpl3::generate_block(int count, T* buffer)
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
bool NukedOpl3::generate(int count, T* buffer)
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

Opl3UPtr make_nuked_opl3()
{
	return std::make_unique<NukedOpl3>();
}

} // bstone
