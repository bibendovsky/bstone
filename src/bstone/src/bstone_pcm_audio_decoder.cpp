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
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int frame_count) override;
	bool rewind() override;

private:
	inline static constexpr int channel_count = 1;

	bool is_initialized_{};
	const char* error_message_{};
	const std::uint8_t* src_data_{};
	int src_size_{};
	int dst_rate_{};
	int counter_{};
	int src_offset_{};
	float sample_{};

	void impl_rewind();
	void set_error_message(const char* error_message);
};

// -------------------------------------

bool PcmAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	terminate();
	if (param.src_raw_data == nullptr)
	{
		set_error_message("No source data.");
		return false;
	}
	if (param.src_raw_size < 0)
	{
		set_error_message("Invalid source size.");
		return false;
	}
	if (param.dst_rate < 11025)
	{
		set_error_message("Sample rate too small.");
		return false;
	}
	src_data_ = static_cast<const unsigned char*>(param.src_raw_data);
	src_size_ = param.src_raw_size;
	dst_rate_ = param.dst_rate;
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

const char* PcmAudioDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int PcmAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(is_initialized());
	return channel_count;
}

int PcmAudioDecoder::decode_frames(float* samples, int frame_count)
{
	BSTONE_ASSERT(is_initialized());
#if 0 // FIXME
	if (src_offset_ >= total_frames_)
#else
	if (src_offset_ >= src_size_)
#endif
		return 0;
	int i = 0;
	for (; i < frame_count; ++i)
	{
		if (counter_ >= dst_rate_)
		{
			counter_ -= dst_rate_;
			++src_offset_;
			if (src_offset_ >= src_size_)
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

void PcmAudioDecoder::set_error_message(const char* error_message)
{
	error_message_ = error_message;
}

} // namespace

// =====================================

AudioDecoderUPtr make_pcm_audio_decoder()
{
	return std::make_unique<PcmAudioDecoder>();
}

} // namespace bstone
