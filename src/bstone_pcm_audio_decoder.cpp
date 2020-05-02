/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2020 Boris I. Bendovsky (bibendovsky@hotmail.com)

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
// PCM audio decoder.
//


#include "bstone_precompiled.h"

#include "bstone_audio_decoder.h"
#include "bstone_low_pass_filter.h"


namespace bstone
{


//
// PCM audio decoder.
//
class PcmDecoder final :
	public AudioDecoder
{
public:
	PcmDecoder();

	~PcmDecoder() override;

	bool initialize(
		const void* const src_raw_data,
		const int src_raw_size,
		const int dst_rate) override;

	void uninitialize() override;

	bool is_initialized() const noexcept override;

	int decode(
		const int dst_count,
		std::int16_t* const dst_data) override;

	bool rewind() override;

	int get_dst_length_in_samples() const noexcept override;

	// Return an input sample rate.
	static int get_src_rate();

	// Returns a minimum output sample rate.
	static int get_min_dst_rate();


private:
	bool is_initialized_;

	const void* src_raw_data_;

	std::int64_t offset_;
	std::int64_t dst_count_;
	std::int64_t dst_ratio_;

	LowPassFilterInt16 low_pass_filter_;

	int dst_length_in_samples_;


	void uninitialize_internal();

	static std::int16_t pcm_u8_to_pcm_s16(
		const std::uint8_t sample);
}; // PcmDecoder


PcmDecoder::PcmDecoder()
	:
	is_initialized_{},
	src_raw_data_{},
	offset_{},
	dst_count_{},
	dst_ratio_{},
	low_pass_filter_{},
	dst_length_in_samples_{}
{
}

PcmDecoder::~PcmDecoder() = default;

bool PcmDecoder::initialize(
	const void* const src_raw_data,
	const int src_raw_size,
	const int dst_rate)
{
	uninitialize();

	if (!src_raw_data)
	{
		return false;
	}

	if (src_raw_size < 0)
	{
		return false;
	}

	if (dst_rate < 1)
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

	src_raw_data_ = src_raw_data;
	dst_length_in_samples_ = static_cast<int>((i_size * o_rate) / i_rate);

	dst_count_ = get_dst_length_in_samples();
	dst_ratio_ = (i_size * 65'536) / dst_count_;

	low_pass_filter_.initialize(get_src_rate(), dst_rate);

	is_initialized_ = true;

	return true;
}

bool PcmDecoder::is_initialized() const noexcept
{
	return is_initialized_;
}

void PcmDecoder::uninitialize()
{
	uninitialize_internal();
}

int PcmDecoder::decode(
	const int dst_count,
	std::int16_t* const dst_data)
{
	if (!is_initialized_)
	{
		return 0;
	}

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

	const auto src_samples = static_cast<const std::uint8_t*>(src_raw_data_);

	auto offset = offset_;

	auto cached_index = -1;
	auto cached_sample = std::int16_t{};

	for (int i = 0; i < dst_count && offset < dst_count_; ++i)
	{
		const auto src_index = static_cast<int>((offset * dst_ratio_) / 65'536);

		if (src_index != cached_index)
		{
			cached_index = src_index;
			cached_sample = pcm_u8_to_pcm_s16(src_samples[src_index]);
		}

		dst_data[i] = cached_sample;

		offset += 1;
		actual_count += 1;
	}

	offset = offset_;

	for (int i = 0; i < actual_count; ++i)
	{
		dst_data[i] = low_pass_filter_.filter(dst_data[i]);

		offset += 1;
	}

	offset_ = offset;

	return actual_count;
}

bool PcmDecoder::rewind()
{
	if (!is_initialized())
	{
		return false;
	}

	offset_ = 0;

	return true;
}

int PcmDecoder::get_dst_length_in_samples() const noexcept
{
	return dst_length_in_samples_;
}

int PcmDecoder::get_src_rate()
{
	return audio_decoder_pcm_fixed_frequency;
}

int PcmDecoder::get_min_dst_rate()
{
	return 11025;
}

void PcmDecoder::uninitialize_internal()
{
	offset_ = 0;
	dst_count_ = 0;
	dst_ratio_ = 0;
	low_pass_filter_ = {};
}

std::int16_t PcmDecoder::pcm_u8_to_pcm_s16(
	const std::uint8_t sample)
{
	return static_cast<std::int16_t>(((static_cast<int>(sample) * 65535) / 255) - 32768);
}


namespace detail
{


AudioDecoderUPtr make_pcm_audio_decoder()
{
	return std::make_unique<PcmDecoder>();
}


} // detail


} // bstone
