/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

// PCM audio decoder

#include "bstone_pcm_audio_decoder.h"
#include "bstone_assert.h"
#include "bstone_audio_sample_converter.h"
#include <cstdint>

namespace bstone {

namespace {

class PcmAudioDecoder final : public AudioDecoder
{
public:
	PcmAudioDecoder() = default;
	~PcmAudioDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	int get_total_frames() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int frame_count) override;
	bool rewind() override;

private:
	inline static constexpr int channel_count = 1;

	bool is_initialized_{};
	const std::uint8_t* src_data_{};
	int src_size_{};
	int dst_rate_{};
	int total_frames_{};
	int counter_{};
	int src_offset_{};
	float sample_{};

	void impl_rewind();
};

// -------------------------------------

bool PcmAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	terminate();
	if (param.src_raw_data == nullptr)
		return false;
	if (param.src_raw_size < 0)
		return false;
	if (param.dst_rate < 11025)
		return false;
	src_data_ = static_cast<const unsigned char*>(param.src_raw_data);
	src_size_ = param.src_raw_size;
	dst_rate_ = param.dst_rate;
	const long long src_size_ll = src_size_;
	const long long dst_rate_ll = dst_rate_;
	const long long audio_decoder_w3d_pcm_frequency_ll = static_cast<long long>(audio_decoder_w3d_pcm_frequency);
	total_frames_ =
		static_cast<int>(((src_size_ll * dst_rate_ll) + audio_decoder_w3d_pcm_frequency_ll - 1) / audio_decoder_w3d_pcm_frequency_ll);
	impl_rewind();
	is_initialized_ = true;
	return true;
}

void PcmAudioDecoder::terminate()
{
	is_initialized_ = false;
}

bool PcmAudioDecoder::is_initialized() const
{
	return is_initialized_;
}

int PcmAudioDecoder::get_total_frames() const
{
	BSTONE_ASSERT(is_initialized());
	return total_frames_;
}

int PcmAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

int PcmAudioDecoder::decode_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
	if (src_offset_ >= total_frames_)
		return 0;
	int i = 0;
	for (; i < frame_count; ++i)
	{
		if (counter_ >= dst_rate_)
		{
			counter_ -= dst_rate_;
			++src_offset_;
			if (src_offset_ >= total_frames_)
				break;
			sample_ = AudioSampleConverter::u8_to_f32(src_data_[src_offset_]);
		}
		counter_ += audio_decoder_w3d_pcm_frequency;
		samples[i] = sample_;
	}
	return i;
}

bool PcmAudioDecoder::rewind()
{
	BSTONE_ASSERT(is_initialized());
	impl_rewind();
	return true;
}

void PcmAudioDecoder::impl_rewind()
{
	counter_ = dst_rate_;
	src_offset_ = -1;
	sample_ = 0.0F;
}

} // namespace

// =====================================

AudioDecoderUPtr make_pcm_audio_decoder()
{
	return std::make_unique<PcmAudioDecoder>();
}

} // namespace bstone
