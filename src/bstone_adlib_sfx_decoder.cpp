/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2015 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// A decoder for AdLib sound effects.
//


#include "bstone_adlib_sfx_decoder.h"
#include "bstone_endian.h"


namespace bstone
{


void AdlibSfxDecoder::Instrument::reset()
{
	m_char_ = 0;
	c_char_ = 0;
	m_scale_ = 0;
	c_scale_ = 0;
	m_attack_ = 0;
	c_attack_ = 0;
	m_sus_ = 0;
	c_sus_ = 0;
	m_wave_ = 0;
	c_wave_ = 0;
}

AdlibSfxDecoder::AdlibSfxDecoder() :
	commands_count_{},
	command_index_{},
	samples_per_tick_{},
	remains_count_{},
	hf_{}
{
}

AdlibSfxDecoder::~AdlibSfxDecoder()
{
	uninitialize_internal();
}

bool AdlibSfxDecoder::initialize(
	const void* const raw_data,
	const int raw_size,
	const int dst_rate)
{
	if (!AdlibDecoder::initialize(raw_data, raw_size, dst_rate))
	{
		return false;
	}

	static_cast<void>(reader_.open(raw_data, raw_size));

	const auto& endian = bstone::Endian{};

	const auto sfx_length = static_cast<int>(endian.little(reader_.read_s32()));

	if (sfx_length <= 0)
	{
		return false;
	}

	if ((sfx_length + get_header_size()) >= raw_size)
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

	initialize_instrument();

	command_index_ = 0;
	commands_count_ = sfx_length;
	samples_per_tick_ = emulator_.get_sample_rate() / get_tick_rate();
	set_dst_length_in_samples(samples_per_tick_ * sfx_length);
	remains_count_ = 0;

	set_is_initialized(true);

	return true;
}

void AdlibSfxDecoder::uninitialize()
{
	uninitialize_internal();

	AdlibDecoder::uninitialize();
}

bool AdlibSfxDecoder::reset()
{
	if (!AdlibDecoder::reset())
	{
		return false;
	}

	initialize_instrument();

	command_index_ = 0;
	remains_count_ = 0;

	reader_.set_position(get_header_size());

	return true;
}

AudioDecoder* AdlibSfxDecoder::clone()
{
	return new AdlibSfxDecoder(*this);
}

int AdlibSfxDecoder::decode(
	const int dst_count,
	std::int16_t* const dst_data)
{
	if (!is_initialized())
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

			emulator_.generate(count, &dst_data[dst_data_index]);

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
					emulator_.write(al_freq_l, lf);
					emulator_.write(al_freq_h, hf_);
				}
				else
				{
					emulator_.write(al_freq_h, 0x00);
				}

				++command_index_;

				remains_count_ = samples_per_tick_;
			}
		}

		quit = ((command_index_ == commands_count_ && remains_count_ == 0) || dst_remain_count == 0);
	}

	return decoded_samples_count;
}

void AdlibSfxDecoder::uninitialize_internal()
{
	reader_.close();
	instrument_.reset();
	commands_count_ = 0;
	command_index_ = 0;
	samples_per_tick_ = 0;
	remains_count_ = 0;
	hf_ = 0;
}

void AdlibSfxDecoder::initialize_instrument()
{
	// carrier
	const auto c = 3;

	// modifier
	const auto m = 0;

	emulator_.write(m + al_char, instrument_.m_char_);
	emulator_.write(m + al_scale, instrument_.m_scale_);
	emulator_.write(m + al_attack, instrument_.m_attack_);
	emulator_.write(m + al_sus, instrument_.m_sus_);
	emulator_.write(m + al_wave, instrument_.m_wave_);
	emulator_.write(c + al_char, instrument_.c_char_);
	emulator_.write(c + al_scale, instrument_.c_scale_);
	emulator_.write(c + al_attack, instrument_.c_attack_);
	emulator_.write(c + al_sus, instrument_.c_sus_);
	emulator_.write(c + al_wave, instrument_.c_wave_);

	emulator_.write(al_feed_con, 0);
}

int AdlibSfxDecoder::get_tick_rate()
{
	return 140;
}

int AdlibSfxDecoder::get_header_size()
{
	return 23;
}


} // bstone
