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
// A PCM decoder for digitized sounds.
//


#include "bstone_pcm_decoder.h"
#include <cstddef>


namespace bstone
{


PcmDecoder::PcmDecoder() :
	offset_{},
	last_sample_{},
	dst_count_{},
	dst_ratio_{},
	alpha_{},
	one_minus_alpha_{}
{
}

PcmDecoder::~PcmDecoder()
{
	uninitialize_internal();
}

bool PcmDecoder::initialize(
	const void* const src_raw_data,
	const int src_raw_size,
	const int dst_rate)
{
	if (!AudioDecoder::initialize(src_raw_data, src_raw_size, dst_rate))
	{
		return false;
	}

	if (dst_rate < get_min_dst_rate())
	{
		return false;
	}

	const auto i_size = static_cast<std::int64_t>(src_raw_size);
	const auto o_rate = static_cast<std::int64_t>(dst_rate);
	const auto i_rate = static_cast<std::int64_t>(get_src_rate());

	set_dst_length_in_samples(static_cast<int>((i_size * o_rate) / i_rate));

	dst_count_ = get_dst_length_in_samples();
	dst_ratio_ = (i_size << 16) / dst_count_;

	//
	// Calculate parameters of low-pass filter for a cutoff frequency 'f'.
	//
	// For sampling rate r:
	// dt = 1 / r
	// rc = 1 / 2 * PI * f
	// alpha = dt / (rc + dt)
	//

	const auto pi = 3.1415926535897931;
	const auto dt = 1.0 / dst_rate;

	// Our cutoff frequency is half of source rate,
	// because it's a maximum frequency allowed by Nyquist.
	const auto rc = 1.0 / (pi * get_src_rate());

	alpha_ = dt / (rc + dt);
	one_minus_alpha_ = 1.0 - alpha_;

	set_is_initialized(true);

	return true;
}

void PcmDecoder::uninitialize()
{
	uninitialize_internal();

	AudioDecoder::uninitialize();
}

int PcmDecoder::decode(
	const int dst_count,
	std::int16_t* const dst_data)
{
	if (dst_count <= 0)
	{
		return 0;
	}

	if (!dst_data)
	{
		return 0;
	}

	if (offset_ >= dst_count_)
	{
		return 0;
	}

	auto actual_count = 0;

	const auto src_samples = static_cast<const std::uint8_t*>(get_raw_data());

	auto offset = offset_;

	auto cached_index = -1;
	auto cached_sample = std::int16_t{};

	for (int i = 0; i < dst_count && offset < dst_count_; ++i)
	{
		const auto src_index = static_cast<int>((offset * dst_ratio_) >> 16);

		if (src_index != cached_index)
		{
			cached_index = src_index;
			cached_sample = pcm8_to_pcm16(src_samples[src_index]);
		}

		dst_data[i] = cached_sample;

		++offset;
		++actual_count;
	}

	offset = offset_;

	for (int i = 0; i < actual_count; ++i)
	{
		if (!(i == 0 && offset == 0))
		{
			auto prev_sample = std::int16_t{};

			if (i > 0)
			{
				prev_sample = dst_data[i - 1];
			}
			else
			{
				prev_sample = last_sample_;
			}

			dst_data[i] = static_cast<std::int16_t>((alpha_ * dst_data[i]) + (one_minus_alpha_ * prev_sample));
		}

		++offset;
	}

	offset_ = offset;
	last_sample_ = dst_data[actual_count - 1];

	return actual_count;
}

bool PcmDecoder::reset()
{
	if (!is_initialized())
	{
		return false;
	}

	offset_ = 0;

	return true;
}

AudioDecoder* PcmDecoder::clone()
{
	return new PcmDecoder(*this);
}

int PcmDecoder::get_src_rate()
{
	return 7000;
}

int PcmDecoder::get_min_dst_rate()
{
	return 11025;
}

void PcmDecoder::uninitialize_internal()
{
	offset_ = 0;
	last_sample_ = 0;
	dst_count_ = 0;
	dst_ratio_ = 0;
	alpha_ = 0.0;
	one_minus_alpha_ = 0.0;
}

std::int16_t PcmDecoder::pcm8_to_pcm16(
	const std::uint8_t sample)
{
	return static_cast<std::int16_t>(((static_cast<int>(sample) * 65535) / 255) - 32768);
}


} // bstone
