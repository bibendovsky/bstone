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

#include <cassert>
#include <cmath>

#include <algorithm>
#include <type_traits>
#include <vector>

#include "bstone_audio_decoder.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_low_pass_filter.h"
#include "bstone_unique_resource.h"


namespace bstone
{


namespace detail
{


template<typename TSrc, typename TDst>
struct PcmDecoderSampleConverter;

template<>
struct PcmDecoderSampleConverter<std::uint8_t, std::int16_t>
{
	std::int16_t operator()(
		const std::uint8_t u8_sample) const noexcept
	{
		return AudioSampleConverter::u8_to_s16(u8_sample);
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<std::uint8_t, float>
{
	float operator()(
		const std::uint8_t u8_sample) const noexcept
	{
		return AudioSampleConverter::u8_to_f32(u8_sample);
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<std::uint8_t, double>
{
	double operator()(
		const std::uint8_t u8_sample) const noexcept
	{
		return AudioSampleConverter::u8_to_f64(u8_sample);
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<float, std::int16_t>
{
	std::int16_t operator()(
		const float f32_sample) const noexcept
	{
		return AudioSampleConverter::f32_to_s16(f32_sample);
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<float, float>
{
	float operator()(
		const float f32_sample) const noexcept
	{
		return f32_sample;
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<double, std::int16_t>
{
	std::int16_t operator()(
		const double f64_sample) const noexcept
	{
		return AudioSampleConverter::f64_to_s16(f64_sample);
	}
}; // PcmDecoderSampleConverter

template<>
struct PcmDecoderSampleConverter<double, float>
{
	float operator()(
		const double f64_sample) const noexcept
	{
		return static_cast<float>(f64_sample);
	}
}; // PcmDecoderSampleConverter


} // detail


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

	int decode(
		const int dst_count,
		float* const dst_data) override;

	bool rewind() override;

	int get_dst_length_in_samples() const noexcept override;

	// Return an input sample rate.
	static int get_src_rate();

	// Returns a minimum output sample rate.
	static int get_min_dst_rate();


private:
	static constexpr auto lpf_order =
#ifdef NDEBUG
		80
#else
		40
#endif
	;


	bool is_initialized_;
	bool need_upsampling_;

	double ratio_;

	const unsigned char* src_data_;

	int src_offset_;
	int src_count_;

	int dst_offset_;
	int dst_count_;

	LowPassFilter lpf_;


	void uninitialize_internal();

	template<typename TDst>
	int decode_upsampled(
		const int dst_count,
		TDst* const dst_data);

	template<typename TSrc, typename TDst>
	int decode_non_upsampled(
		const int dst_count,
		TDst* const dst_data);

	template<typename T>
	int decode(
		const int dst_count,
		T* const dst_data);
}; // PcmDecoder


PcmDecoder::PcmDecoder()
	:
	is_initialized_{},
	need_upsampling_{},
	ratio_{},
	src_data_{},
	src_offset_{},
	src_count_{},
	dst_offset_{},
	dst_count_{},
	lpf_{}
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

	need_upsampling_ = (dst_rate != get_src_rate());

	ratio_ = static_cast<double>(dst_rate) / static_cast<double>(get_src_rate());

	src_data_ = static_cast<const unsigned char*>(src_raw_data);
	src_offset_ = 0;
	src_count_ = src_raw_size;

	dst_offset_ = 0;
	dst_count_ = static_cast<int>(std::ceil(src_raw_size * ratio_));

	if (need_upsampling_)
	{
		lpf_.initialize(lpf_order, get_src_rate() / 2, dst_rate);
	}

	is_initialized_ = true;

	return is_initialized_;
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
	return decode<std::int16_t>(dst_count, dst_data);
}

int PcmDecoder::decode(
	const int dst_count,
	float* const dst_data)
{
	return decode<float>(dst_count, dst_data);
}

bool PcmDecoder::rewind()
{
	if (!is_initialized())
	{
		return false;
	}

	src_offset_ = 0;
	dst_offset_ = 0;

	return true;
}

int PcmDecoder::get_dst_length_in_samples() const noexcept
{
	return dst_count_;
}

int PcmDecoder::get_src_rate()
{
	return audio_decoder_pcm_fixed_frequency;
}

int PcmDecoder::get_min_dst_rate()
{
	return 11'025;
}

void PcmDecoder::uninitialize_internal()
{
	is_initialized_ = false;
	need_upsampling_ = false;
	ratio_ = 0.0;
	src_data_ = nullptr;
	src_offset_ = 0;
	src_count_ = 0;
	dst_offset_ = 0;
	dst_count_ = 0;
}

template<typename TDst>
int PcmDecoder::decode_upsampled(
	const int dst_count,
	TDst* const dst_data)
{
	const auto u8_to_xx = detail::PcmDecoderSampleConverter<std::uint8_t, TDst>{};
	const auto u8_to_f64 = detail::PcmDecoderSampleConverter<std::uint8_t, double>{};
	const auto f64_to_xx = detail::PcmDecoderSampleConverter<double, TDst>{};

	const auto count = std::min(dst_count, dst_count_ - dst_offset_);

	for (int i = 0; i < count; ++i)
	{
		const auto src_offset = static_cast<int>(
			(static_cast<long long>(src_count_ - 1) * static_cast<long long>(dst_offset_)) /
				static_cast<long long>(dst_count_ - 1)
		);

		const auto u8_sample = src_data_[src_offset];
		const auto f64_sample = u8_to_f64(u8_sample);
		const auto f64_lpf_sample = lpf_.process_sample(f64_sample);
		const auto dst_sample = f64_to_xx(f64_lpf_sample);

		dst_data[i] = dst_sample;

		dst_offset_ += 1;
	}

	return count;
}

template<typename TSrc, typename TDst>
int PcmDecoder::decode_non_upsampled(
	const int dst_count,
	TDst* const dst_data)
{
	if (src_offset_ >= src_count_)
	{
		return 0;
	}

	const auto to_copy_count = std::min(
		dst_count,
		src_count_ - src_offset_
	);

	std::transform(
		&src_data_[src_offset_],
		&src_data_[src_offset_ + to_copy_count],
		dst_data,
		detail::PcmDecoderSampleConverter<std::uint8_t, TDst>{}
	);

	src_offset_ += to_copy_count;
	dst_offset_ += to_copy_count;

	return to_copy_count;
}

template<typename T>
int PcmDecoder::decode(
	const int dst_count,
	T* const dst_data)
{
	if (!is_initialized_)
	{
		return 0;
	}

	if (dst_count <= 0)
	{
		return 0;
	}

	if (dst_data == nullptr)
	{
		return 0;
	}

	if (dst_offset_ >= dst_count_)
	{
		return 0;
	}

	if (need_upsampling_)
	{
		return decode_upsampled<T>(dst_count, dst_data);
	}
	else
	{
		return decode_non_upsampled<std::uint8_t, T>(dst_count, dst_data);
	}
}


namespace detail
{


AudioDecoderUPtr make_pcm_audio_decoder()
{
	return std::make_unique<PcmDecoder>();
}


} // detail


} // bstone
