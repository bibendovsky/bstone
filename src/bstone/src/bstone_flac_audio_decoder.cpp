#include "bstone_flac_audio_decoder.h"
#include "bstone_assert.h"
#include <cstddef>
#include <algorithm>
#include <utility>
#include <vector>
#include "dr_flac.h"

namespace bstone {

namespace {

class FlacAudioDecoder final : public AudioDecoder
{
public:
	FlacAudioDecoder() = default;
	~FlacAudioDecoder() override;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int max_frames) override;
	bool rewind() override;

private:
	inline static constexpr int cache_capacity = 4096;

	using DecodeFramesFunc = int (FlacAudioDecoder::*)(float* samples, int max_frames);

	const char* error_message_{};
	drflac* flac_{};
	// dr_flac references this buffer for its whole lifetime, so it must outlive flac_.
	std::vector<unsigned char> data_{};
	bool is_open_{};
	int bit_depth_{};
	int channel_count_{};
	int src_frame_rate_{};
	int dst_frame_rate_{};
	// Interleaved, normalised float frames; only used by the resampling path.
	std::vector<float> cache_{};
	int cache_frame_count_{};
	int cache_frame_offset_{};
	int cache_frame_offset_counter_{};
	DecodeFramesFunc decode_frames_{};

	void set_error_message(const char* message);

	bool impl_is_initialized() const;
	void impl_close_flac();
	bool impl_open_flac();
	void impl_terminate();
	bool impl_initialize(const AudioDecoderInitParam& param);
	bool update_cache();
	int decode_frames_as_is(float* samples, int max_frames);
	int decode_frames_with_resample(float* samples, int max_frames);
};

// -------------------------------------

FlacAudioDecoder::~FlacAudioDecoder()
{
	impl_close_flac();
}

bool FlacAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	error_message_ = nullptr;
	if (!impl_initialize(param))
	{
		impl_terminate();
		return false;
	}
	return true;
}

void FlacAudioDecoder::terminate()
{
	impl_terminate();
}

bool FlacAudioDecoder::is_initialized() const
{
	return impl_is_initialized();
}

const char* FlacAudioDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int FlacAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(impl_is_initialized());
	return channel_count_;
}

int FlacAudioDecoder::decode_frames(float* samples, int max_frames)
{
	BSTONE_ASSERT(max_frames >= 0);
	BSTONE_ASSERT(impl_is_initialized());
	return (this->*decode_frames_)(samples, max_frames);
}

bool FlacAudioDecoder::rewind()
{
	BSTONE_ASSERT(impl_is_initialized());
	if (drflac_seek_to_pcm_frame(flac_, 0) == DRFLAC_FALSE)
	{
		set_error_message("Failed to rewind a stream.");
		return false;
	}
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	return true;
}

void FlacAudioDecoder::set_error_message(const char* message)
{
	error_message_ = message;
}

bool FlacAudioDecoder::impl_is_initialized() const
{
	return is_open_;
}

void FlacAudioDecoder::impl_close_flac()
{
	if (flac_ != nullptr)
	{
		drflac_close(flac_);
		flac_ = nullptr;
	}
	is_open_ = false;
}

bool FlacAudioDecoder::impl_open_flac()
{
	flac_ = drflac_open_memory(data_.data(), data_.size(), nullptr);
	if (flac_ == nullptr)
	{
		set_error_message("Failed to open a FLAC stream.");
		return false;
	}
	bit_depth_ = static_cast<int>(flac_->bitsPerSample);
	channel_count_ = static_cast<int>(flac_->channels);
	src_frame_rate_ = static_cast<int>(flac_->sampleRate);
	is_open_ = true;
	return true;
}

void FlacAudioDecoder::impl_terminate()
{
	impl_close_flac();
	data_.clear();
	data_.shrink_to_fit();
	cache_.clear();
	cache_.shrink_to_fit();
}

bool FlacAudioDecoder::impl_initialize(const AudioDecoderInitParam& param)
{
	impl_terminate();
	if (param.vfs_stream == nullptr)
	{
		set_error_message("No VFS stream.");
		return false;
	}
	if (param.dst_rate < 1)
	{
		set_error_message("Invalid destination audio frame rate.");
		return false;
	}
	// Read the whole stream into memory up front; dr_flac decodes from the buffer.
	VfsInputStreamUPtr stream = std::move(param.vfs_stream);
	const int stream_size = stream->get_size();
	if (stream_size <= 0)
	{
		set_error_message("Empty or invalid stream.");
		return false;
	}
	data_.resize(static_cast<std::size_t>(stream_size));
	if (!stream->read_exactly(data_.data(), stream_size))
	{
		set_error_message("Failed to read a stream.");
		return false;
	}
	if (!impl_open_flac())
		return false;
	switch (bit_depth_)
	{
		case 8:
		case 16:
		case 24:
		case 32:
			break;
		default:
			set_error_message("Unsupported bit depth.");
			return false;
	}
	switch (channel_count_)
	{
		case 1:
		case 2:
			break;
		default:
			set_error_message("Unsupported channel count.");
			return false;
	}
	if (src_frame_rate_ < audio_decoder_min_sample_rate || src_frame_rate_ > audio_decoder_max_sample_rate)
	{
		set_error_message("Unsupported track's sample rate.");
		return false;
	}
	dst_frame_rate_ = param.dst_rate;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	if (src_frame_rate_ == dst_frame_rate_)
	{
		decode_frames_ = &FlacAudioDecoder::decode_frames_as_is;
	}
	else
	{
		cache_.resize(static_cast<std::size_t>(cache_capacity) * channel_count_);
		decode_frames_ = &FlacAudioDecoder::decode_frames_with_resample;
	}
	return true;
}

bool FlacAudioDecoder::update_cache()
{
	if (cache_frame_offset_ < cache_frame_count_)
		return true;
	const drflac_uint64 read_frame_count = drflac_read_pcm_frames_f32(
		flac_, static_cast<drflac_uint64>(cache_capacity), cache_.data());
	cache_frame_count_ = static_cast<int>(read_frame_count);
	cache_frame_offset_ = 0;
	return cache_frame_count_ > 0;
}

int FlacAudioDecoder::decode_frames_as_is(float* samples, int max_frames)
{
	// dr_flac already yields interleaved, normalised float at the source rate.
	const drflac_uint64 read_frame_count = drflac_read_pcm_frames_f32(
		flac_, static_cast<drflac_uint64>(max_frames), samples);
	return static_cast<int>(read_frame_count);
}

int FlacAudioDecoder::decode_frames_with_resample(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		if (!update_cache())
			break;
		const float* const cache_frame = &cache_[static_cast<std::size_t>(cache_frame_offset_) * channel_count_];
		std::copy_n(cache_frame, channel_count_, samples);
		samples += channel_count_;
		++written_frame_count;
		cache_frame_offset_counter_ += src_frame_rate_;
		while (cache_frame_offset_counter_ >= dst_frame_rate_)
		{
			cache_frame_offset_counter_ -= dst_frame_rate_;
			++cache_frame_offset_;
			if (!update_cache())
				break;
		}
	}
	return written_frame_count;
}

} // namespace

// =====================================

AudioDecoderUPtr make_flac_audio_decoder()
{
	return std::make_unique<FlacAudioDecoder>();
}

} // namespace bstone
