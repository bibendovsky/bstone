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
// A decoder for AdLib music.
//


#include "bstone_adlib_music_decoder.h"
#include "bstone_endian.h"


namespace bstone
{


AdlibMusicDecoder::AdlibMusicDecoder() :
	commands_count_{},
	command_index_{},
	samples_per_tick_{},
	remains_count_{}
{
}

AdlibMusicDecoder::~AdlibMusicDecoder()
{
	uninitialize_internal();
}

bool AdlibMusicDecoder::initialize(
	const void* const raw_data,
	const int raw_size,
	const int dst_rate)
{
	uninitialize();

	if (!AdlibDecoder::initialize(raw_data, raw_size, dst_rate))
	{
		return false;
	}

	static_cast<void>(reader_.open(raw_data, raw_size));

	const auto commands_size = static_cast<int>(bstone::Endian::little(reader_.read_u16()));

	if ((commands_size % 4) != 0)
	{
		return false;
	}

	if ((commands_size + 2) > raw_size)
	{
		return false;
	}

	command_index_ = 0;
	commands_count_ = commands_size / 4;

	samples_per_tick_ = emulator_.get_sample_rate() / get_tick_rate();
	remains_count_ = 0;

	auto ticks_count = 0;

	for (int i = 0; i < commands_count_; ++i)
	{
		reader_.skip(2);
		ticks_count += bstone::Endian::little(reader_.read_u16());
	}

	set_dst_length_in_samples(ticks_count * samples_per_tick_);

	reader_.set_position(2);

	set_is_initialized(true);

	return true;
}

void AdlibMusicDecoder::uninitialize()
{
	uninitialize_internal();

	AdlibDecoder::uninitialize();
}

bool AdlibMusicDecoder::reset()
{
	if (!AdlibDecoder::reset())
	{
		return false;
	}

	reader_.set_position(2);

	command_index_ = 0;
	remains_count_ = 0;

	return true;
}

AudioDecoder* AdlibMusicDecoder::clone()
{
	return new AdlibMusicDecoder(*this);
}

int AdlibMusicDecoder::decode(
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
			int delay = 0;

			while (command_index_ < commands_count_ && delay == 0)
			{
				const auto command_port = static_cast<int>(reader_.read_u8());
				const auto command_value = static_cast<int>(reader_.read_u8());

				delay = bstone::Endian::little(reader_.read_u16());

				emulator_.write(command_port, command_value);
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

int AdlibMusicDecoder::get_tick_rate()
{
	return 700;
}

void AdlibMusicDecoder::uninitialize_internal()
{
	reader_.close();
	commands_count_ = 0;
	command_index_ = 0;
	samples_per_tick_ = 0;
	remains_count_ = 0;
}


} // bstone
