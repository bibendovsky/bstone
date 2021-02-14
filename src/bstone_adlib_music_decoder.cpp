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
// AdLib music decoder.
//


#include <algorithm>

#include "bstone_adlib_decoder.h"
#include "bstone_audio_decoder.h"
#include "bstone_endian.h"
#include "bstone_memory_binary_reader.h"
#include "bstone_opl3.h"


namespace bstone
{


//
// AdLib music decoder.
//
class AdlibMusicDecoder final :
	public AudioDecoder
{
public:
	AdlibMusicDecoder(
		const Opl3Type opl3_type);

	~AdlibMusicDecoder() override;

	bool initialize(
		const AudioDecoderInitParam& param) override;

	void uninitialize() override;

	bool is_initialized() const noexcept override;

	int decode(
		const int dst_count,
		std::int16_t* const dst_data) override;

	int decode(
		const int dst_count,
		float* const dst_data) override;

	bool rewind() override;

	int get_dst_length_in_samples() const noexcept override;

	bool set_resampling(
		const AudioDecoderInterpolationType interpolation_type,
		const bool lpf,
		const bool lpf_flush_samples) override;

	// Returns a number of calls per second of
	// original interrupt routine.
	static int get_tick_rate();


private:
	Opl3UPtr emulator_;

	bool is_initialized_;

	MemoryBinaryReader reader_;
	int commands_count_;
	int command_index_;
	int samples_per_tick_;
	int remains_count_;

	int dst_length_in_samples_;


	void uninitialize_internal();

	template<typename T>
	int decode(
		const int dst_count,
		T* const dst_data);
}; // AdlibMusicDecoder


AdlibMusicDecoder::AdlibMusicDecoder(
	const Opl3Type opl3_type)
	:
	emulator_{make_opl3(opl3_type)},
	is_initialized_{},
	reader_{},
	commands_count_{},
	command_index_{},
	samples_per_tick_{},
	remains_count_{},
	dst_length_in_samples_{}
{
}

AdlibMusicDecoder::~AdlibMusicDecoder() = default;

bool AdlibMusicDecoder::initialize(
	const AudioDecoderInitParam& param)
{
	uninitialize();

	if (!emulator_)
	{
		return false;
	}

	if (!param.src_raw_data_)
	{
		return false;
	}

	if (param.src_raw_size_ < 0)
	{
		return false;
	}

	if (param.dst_rate_ < 1)
	{
		return false;
	}

	emulator_->initialize(param.dst_rate_);

	static_cast<void>(reader_.open(param.src_raw_data_, param.src_raw_size_));

	const auto commands_size = static_cast<int>(bstone::Endian::little(reader_.read_u16()));

	if ((commands_size % 4) != 0)
	{
		return false;
	}

	if ((commands_size + 2) > param.src_raw_size_)
	{
		return false;
	}

	command_index_ = 0;
	commands_count_ = commands_size / 4;

	samples_per_tick_ = emulator_->get_sample_rate() / get_tick_rate();
	remains_count_ = 0;

	auto ticks_count = 0;

	for (int i = 0; i < commands_count_; ++i)
	{
		reader_.skip(2);
		ticks_count += bstone::Endian::little(reader_.read_u16());
	}

	dst_length_in_samples_ = ticks_count * samples_per_tick_;

	reader_.set_position(2);

	is_initialized_ = true;

	return true;
}

bool AdlibMusicDecoder::is_initialized() const noexcept
{
	return is_initialized_;
}

void AdlibMusicDecoder::uninitialize()
{
	uninitialize_internal();
}

int AdlibMusicDecoder::decode(
	const int dst_count,
	float* const dst_data)
{
	return decode<float>(dst_count, dst_data);
}

bool AdlibMusicDecoder::rewind()
{
	if (!emulator_->reset())
	{
		return false;
	}

	adlib::initialize_registers(emulator_.get());

	reader_.set_position(2);

	command_index_ = 0;
	remains_count_ = 0;

	return true;
}

int AdlibMusicDecoder::get_dst_length_in_samples() const noexcept
{
	return dst_length_in_samples_;
}

bool AdlibMusicDecoder::set_resampling(
	const AudioDecoderInterpolationType interpolation_type,
	const bool lpf,
	const bool lpf_flush_samples)
{
	return false;
}

int AdlibMusicDecoder::decode(
	const int dst_count,
	std::int16_t* const dst_data)
{
	return decode<std::int16_t>(dst_count, dst_data);
}

int AdlibMusicDecoder::get_tick_rate()
{
	return 700;
}

void AdlibMusicDecoder::uninitialize_internal()
{
	if (emulator_)
	{
		emulator_->reset();
	}

	is_initialized_ = {};
	reader_.close();
	commands_count_ = {};
	command_index_ = {};
	samples_per_tick_ = {};
	remains_count_ = {};

	dst_length_in_samples_ = {};
}

template<typename T>
int AdlibMusicDecoder::decode(
	const int dst_count,
	T* const dst_data)
{
	if (!is_initialized_)
	{
		return 0;
	}

	if (dst_count < 1)
	{
		return 0;
	}

	if (!dst_data)
	{
		return 0;
	}

	if (command_index_ == commands_count_ && remains_count_ == 0)
	{
		return 0;
	}

	auto decoded_samples_count = 0;

	auto dst_data_index = 0;
	auto dst_remain_count = dst_count;

	for (bool quit = false; !quit; )
	{
		if (remains_count_ > 0)
		{
			int count = std::min(dst_remain_count, remains_count_);

			emulator_->generate(count, &dst_data[dst_data_index]);

			dst_data_index += count;
			dst_remain_count -= count;
			remains_count_ -= count;
			decoded_samples_count += count;
		}
		else
		{
			int delay = 0;

			while (command_index_ < commands_count_ && delay == 0)
			{
				const auto command_port = static_cast<int>(reader_.read_u8());
				const auto command_value = static_cast<int>(reader_.read_u8());

				delay = bstone::Endian::little(reader_.read_u16());

				emulator_->write(command_port, command_value);
				++command_index_;
			}

			if (delay > 0)
			{
				remains_count_ = delay * samples_per_tick_;
			}
		}

		quit = ((command_index_ == commands_count_ && remains_count_ == 0) || dst_remain_count == 0);
	}

	return decoded_samples_count;
}


namespace detail
{


AudioDecoderUPtr make_adlib_music_audio_decoder(
	const Opl3Type opl3_type)
{
	return std::make_unique<AdlibMusicDecoder>(opl3_type);
}


} // detail


} // bstone
