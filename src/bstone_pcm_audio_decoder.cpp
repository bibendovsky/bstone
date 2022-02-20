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
// PCM audio decoder.
//

#include "bstone_pcm_audio_decoder.h"
#include <cassert>
#include <cstdint>
#include "bstone_audio_sample_converter.h"

namespace bstone
{

namespace
{

// PCM audio decoder.
class PcmAudioDecoder final : public AudioDecoder
{
public:
	PcmAudioDecoder() noexcept;

	bool initialize(const AudioDecoderInitParam& param) override;
	void uninitialize() override;
	bool is_initialized() const noexcept override;
	int decode(int dst_count, std::int16_t* dst_data) override;
	bool rewind() override;
	int get_dst_length_in_samples() const noexcept override;

private:
	bool is_initialized_{};
	const std::uint8_t* src_data_{};
	int src_size_{};
	int dst_rate_{};
	int dst_sample_count_{};
	int counter_{};
	int src_offset_{};
	std::int16_t sample_{};
}; // PcmAudioDecoder

// --------------------------------------------------------------------------

PcmAudioDecoder::PcmAudioDecoder() noexcept = default;

bool PcmAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	uninitialize();

	if (!param.src_raw_data_)
	{
		assert(false && "Null data.");
		return false;
	}

	if (param.src_raw_size_ < 0)
	{
		assert(false && "Data size out of range.");
		return false;
	}

	if (param.dst_rate_ < 11'025)
	{
		assert(false && "Destination rate out of range.");
		return false;
	}

	is_initialized_ = true;
	src_data_ = static_cast<const unsigned char*>(param.src_raw_data_);
	src_size_ = param.src_raw_size_;
	dst_rate_ = param.dst_rate_;
	const auto src_size_ll = static_cast<long long>(src_size_);
	const auto dst_rate_ll = static_cast<long long>(dst_rate_);
	const auto audio_decoder_w3d_pcm_frequency_ll = static_cast<long long>(audio_decoder_w3d_pcm_frequency);
	dst_sample_count_ = static_cast<int>(((src_size_ll * dst_rate_ll) + audio_decoder_w3d_pcm_frequency_ll - 1) / audio_decoder_w3d_pcm_frequency_ll);
	rewind();

	return true;
}

void PcmAudioDecoder::uninitialize()
{
	is_initialized_ = false;
	src_data_ = nullptr;
	src_size_ = 0;
	dst_rate_ = 0;
	dst_sample_count_ = 0;
	counter_ = 0;
	src_offset_ = 0;
	sample_ = 0;
}

bool PcmAudioDecoder::is_initialized() const noexcept
{
	return is_initialized_;
}

int PcmAudioDecoder::decode(int dst_count, std::int16_t* dst_data)
{
	if (dst_count < 0)
	{
		assert(false && "Destination count out of range.");
		return 0;
	}

	if (dst_data == nullptr)
	{
		assert(false && "Null destination data.");
		return 0;
	}

	if (dst_count == 0 || src_offset_ >= dst_sample_count_)
	{
		return 0;
	}

	auto i = 0;

	for (; i < dst_count; ++i)
	{
		if (counter_ >= dst_rate_)
		{
			counter_ -= dst_rate_;
			src_offset_ += 1;

			if (src_offset_ >= dst_sample_count_)
			{
				break;
			}

			sample_ = AudioSampleConverter::u8_to_s16(src_data_[src_offset_]);
		}

		counter_ += audio_decoder_w3d_pcm_frequency;

		dst_data[i] = sample_;
	}

	return i;
}

bool PcmAudioDecoder::rewind()
{
	if (!is_initialized())
	{
		return false;
	}

	counter_ = dst_rate_;
	src_offset_ = -1;
	sample_ = 0;
	return true;
}

int PcmAudioDecoder::get_dst_length_in_samples() const noexcept
{
	return dst_sample_count_;
}

} // namespace

// ==========================================================================

AudioDecoderUPtr make_pcm_audio_decoder()
{
	return std::make_unique<PcmAudioDecoder>();
}

} // bstone
