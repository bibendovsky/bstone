/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


//
// A wrapper for Nuked OPL3.
//


#include <type_traits>
#include <vector>

#include "lib/nuked_opl3/opl3.h"
#include "lib/nuked_opl3/opl3.c"

#include "bstone_audio_sample_converter.h"
#include "bstone_opl3.h"


namespace bstone
{


//
// A wrapper for Nuked OPL3.
//
class NukedOpl3 final :
	public Opl3
{
public:
	Opl3Type get_type() const noexcept override;

	// Initializes the emulator with a specified output sample rate.
	void initialize(
		int sample_rate) override;

	// Uninitializes the emulator.
	void uninitialize() override;

	// Returns true if the wrapper initialized or false otherwise.
	bool is_initialized() const noexcept override;

	// Returns an output sample rate.
	int get_sample_rate() const noexcept override;

	// Writes a value into a register.
	void write(
		int fm_port,
		int fm_value) override;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	bool generate(
		int count,
		std::int16_t* buffer) override;

	// Generates number of mono samples into a provided buffer.
	// Returns false on error.
	bool generate(
		int count,
		float* buffer) override;

	// Resets the emulator.
	bool reset() override;

	// Returns a minimum output sample rate.
	// (Emulator depandant value)
	int get_min_sample_rate() const noexcept override;


private:
	struct S16Tag{};
	struct F32Tag{};

	using Buffer = std::vector<std::int16_t>;


	bool is_initialized_{};
	int sample_rate_{};
	::opl3_chip emulator_{};
	Buffer samples_{};


	// Returns a maximum number of output samples generated at once.
	// (Emulator dependent value)
	static int get_max_samples_count() noexcept;


	void generate_block(
		int count,
		std::int16_t* buffer,
		S16Tag);

	void generate_block(
		int count,
		float* buffer,
		F32Tag);

	template<
		typename T
	>
	void generate_block(
		int count,
		T* buffer);


	template<
		typename T
	>
	bool generate(
		int count,
		T* buffer);
}; // NukedOpl3


Opl3Type NukedOpl3::get_type() const noexcept
{
	return Opl3Type::nuked;
}

void NukedOpl3::initialize(
	int sample_rate)
{
	uninitialize();

	sample_rate_ = std::max(sample_rate, get_min_sample_rate());

	// The emulator outputs stereo samples.
	const auto buffer_size = get_max_samples_count() * 2;

	samples_.resize(buffer_size);

	::OPL3_Reset(&emulator_, static_cast<uint32_t>(sample_rate_));

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

void NukedOpl3::write(
	int fm_port,
	int fm_value)
{
	if (!is_initialized_)
	{
		return;
	}

	::OPL3_WriteReg(&emulator_, static_cast<uint16_t>(fm_port), static_cast<uint8_t>(fm_value));
}

bool NukedOpl3::generate(
	int count,
	std::int16_t* buffer)
{
	return generate<std::int16_t>(count, buffer);
}

bool NukedOpl3::generate(
	int count,
	float* buffer)
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

void NukedOpl3::generate_block(
	int count,
	std::int16_t* buffer,
	const S16Tag)
{
	::OPL3_GenerateStream(&emulator_, samples_.data(), static_cast<uint32_t>(count));

	auto src_samples = samples_.data();

	for (auto i = 0; i < count; ++i)
	{
		const auto src_left_sample = static_cast<int>(*src_samples++);
		const auto src_right_sample = static_cast<int>(*src_samples++);
		const auto src_sample = static_cast<int16_t>((src_left_sample + src_right_sample) / 2);

		buffer[i] = src_sample;
	}
}

void NukedOpl3::generate_block(
	int count,
	float* buffer,
	const F32Tag)
{
	::OPL3_GenerateStream(&emulator_, samples_.data(), static_cast<uint32_t>(count));

	auto src_samples = samples_.data();

	for (auto i = 0; i < count; ++i)
	{
		const auto src_left_sample = static_cast<int>(*src_samples++);
		const auto src_right_sample = static_cast<int>(*src_samples++);
		const auto src_sample = static_cast<int16_t>((src_left_sample + src_right_sample) / 2);

		buffer[i] = AudioSampleConverter::s16_to_f32(src_sample);
	}
}

template<typename T>
void NukedOpl3::generate_block(
	int count,
	T* buffer)
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
bool NukedOpl3::generate(
	int count,
	T* buffer)
{
	if (!is_initialized_)
	{
		return false;
	}

	if (count < 1)
	{
		return false;
	}

	if (buffer == nullptr)
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


namespace detail
{


Opl3UPtr make_nuked_opl3()
{
	return std::make_unique<NukedOpl3>();
}


} // detail


} // bstone
