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
// AdLib sound effects decoder.
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
// AdLib sound effects decoder.
//
class AdlibSfxDecoder final :
	public AudioDecoder
{
public:
	AdlibSfxDecoder(
		const Opl3Type opl3_type);

	~AdlibSfxDecoder() override;

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
	adlib::Instrument instrument_;
	int commands_count_;
	int command_index_;
	int samples_per_tick_;
	int remains_count_;
	int hf_;

	int dst_length_in_samples_;


	void uninitialize_internal();

	// Returns an original size of an AdLibSound structure.
	static int get_header_size();

	template<typename T>
	int decode(
		const int dst_count,
		T* const dst_data);
}; // AdlibDecoder


AdlibSfxDecoder::AdlibSfxDecoder(
	const Opl3Type opl3_type)
	:
	emulator_{make_opl3(opl3_type)},
	is_initialized_{},
	reader_{},
	instrument_{},
	commands_count_{},
	command_index_{},
	samples_per_tick_{},
	remains_count_{},
	hf_{},
	dst_length_in_samples_{}
{
}

AdlibSfxDecoder::~AdlibSfxDecoder()
{
	uninitialize_internal();
}

bool AdlibSfxDecoder::initialize(
	const AudioDecoderInitParam& param)
{
	uninitialize_internal();

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

	adlib::initialize_registers(emulator_.get());

	static_cast<void>(reader_.open(param.src_raw_data_, param.src_raw_size_));

	const auto sfx_length = static_cast<int>(bstone::Endian::little(reader_.read_s32()));

	if (sfx_length <= 0)
	{
		return false;
	}

	if ((sfx_length + get_header_size()) >= param.src_raw_size_)
	{
		return false;
	}

	// Skip priority.
	reader_.skip(2);

	instrument_.m_char_ = reader_.read_u8();
	instrument_.c_char_ = reader_.read_u8();
	instrument_.m_scale_ = reader_.read_u8();
	instrument_.c_scale_ = reader_.read_u8();
	instrument_.m_attack_ = reader_.read_u8();
	instrument_.c_attack_ = reader_.read_u8();
	instrument_.m_sus_ = reader_.read_u8();
	instrument_.c_sus_ = reader_.read_u8();
	instrument_.m_wave_ = reader_.read_u8();
	instrument_.c_wave_ = reader_.read_u8();

	// Skip nConn, voice, mode and 3 unused octets
	reader_.skip(6);

	if (instrument_.m_sus_ == 0 && instrument_.c_sus_ == 0)
	{
		return false;
	}

	hf_ = reader_.read_u8();
	hf_ = ((hf_ & 7) << 2) | 0x20;

	adlib::set_instrument(emulator_.get(), instrument_);

	command_index_ = 0;
	commands_count_ = sfx_length;
	samples_per_tick_ = emulator_->get_sample_rate() / get_tick_rate();
	dst_length_in_samples_ = samples_per_tick_ * sfx_length;
	remains_count_ = 0;

	is_initialized_ = true;

	return true;
}

void AdlibSfxDecoder::uninitialize()
{
	uninitialize_internal();
}

bool AdlibSfxDecoder::rewind()
{
	if (!emulator_->reset())
	{
		return false;
	}

	adlib::initialize_registers(emulator_.get());
	adlib::set_instrument(emulator_.get(), instrument_);

	command_index_ = 0;
	remains_count_ = 0;

	reader_.set_position(get_header_size());

	return true;
}

int AdlibSfxDecoder::get_dst_length_in_samples() const noexcept
{
	return dst_length_in_samples_;
}

bool AdlibSfxDecoder::set_resampling(
	const AudioDecoderInterpolationType interpolation_type,
	const bool lpf,
	const bool lpf_flush_samples)
{
	return false;
}

bool AdlibSfxDecoder::is_initialized() const noexcept
{
	return is_initialized_;
}

int AdlibSfxDecoder::decode(
	const int dst_count,
	std::int16_t* const dst_data)
{
	return decode<std::int16_t>(dst_count, dst_data);
}

int AdlibSfxDecoder::decode(
	const int dst_count,
	float* const dst_data)
{
	return decode<float>(dst_count, dst_data);
}

void AdlibSfxDecoder::uninitialize_internal()
{
	if (emulator_)
	{
		emulator_->reset();
	}

	reader_.close();
	instrument_ = {};
	commands_count_ = {};
	command_index_ = {};
	samples_per_tick_ = {};
	remains_count_ = {};
	hf_ = {};
	dst_length_in_samples_ = {};
}

int AdlibSfxDecoder::get_tick_rate()
{
	return 140;
}

int AdlibSfxDecoder::get_header_size()
{
	return 23;
}

template<typename T>
int AdlibSfxDecoder::decode(
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
			if (command_index_ < commands_count_)
			{
				int lf = reader_.read_u8();

				if (lf > 0)
				{
					emulator_->write(adlib::al_freq_l, lf);
					emulator_->write(adlib::al_freq_h, hf_);
				}
				else
				{
					emulator_->write(adlib::al_freq_h, 0x00);
				}

				++command_index_;

				remains_count_ = samples_per_tick_;
			}
		}

		quit = ((command_index_ == commands_count_ && remains_count_ == 0) || dst_remain_count == 0);
	}

	return decoded_samples_count;
}


namespace detail
{


AudioDecoderUPtr make_adlib_sfx_audio_decoder(
	const Opl3Type opl3_type)
{
	return std::make_unique<AdlibSfxDecoder>(opl3_type);
}


} // detail


} // bstone
