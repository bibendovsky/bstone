#include "bstone_flac_audio_decoder.h"
#include "bstone_assert.h"
#include <cstddef>
#include <cstdint>
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
	inline static constexpr int skip_buffer_size = 4096;

	using DecodeFramesFunc = int (FlacAudioDecoder::*)(float* samples, int max_frames);

	const char* error_message_{};
	drflac* flac_{};
	VfsInputStreamUPtr stream_{};
	std::int64_t stream_position_{};
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

	static std::size_t flac_callback_read(void* user_data, void* buffer, std::size_t size);
	static drflac_bool32 flac_callback_seek(void* user_data, int offset, drflac_seek_origin origin);
	static drflac_bool32 flac_callback_tell(void* user_data, drflac_int64* position);
	std::size_t impl_callback_read(void* buffer, std::size_t size);
	bool impl_callback_seek(int offset, drflac_seek_origin origin);
	bool impl_skip(std::int64_t size);

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

std::size_t FlacAudioDecoder::flac_callback_read(void* user_data, void* buffer, std::size_t size)
{
	return static_cast<FlacAudioDecoder*>(user_data)->impl_callback_read(buffer, size);
}

drflac_bool32 FlacAudioDecoder::flac_callback_seek(void* user_data, int offset, drflac_seek_origin origin)
{
	return static_cast<FlacAudioDecoder*>(user_data)->impl_callback_seek(offset, origin) ? DRFLAC_TRUE : DRFLAC_FALSE;
}

drflac_bool32 FlacAudioDecoder::flac_callback_tell(void* user_data, drflac_int64* position)
{
	*position = static_cast<drflac_int64>(static_cast<FlacAudioDecoder*>(user_data)->stream_position_);
	return DRFLAC_TRUE;
}

std::size_t FlacAudioDecoder::impl_callback_read(void* buffer, std::size_t size)
{
	constexpr std::size_t max_read_size = 1 << 30;
	const int clamped_size = static_cast<int>(std::min(size, max_read_size));
	const auto bytes = static_cast<unsigned char*>(buffer);
	// dr_flac takes a short read for the end of the stream, so fill the whole
	// request from however many reads it takes.
	int bytes_offset = 0;
	while (bytes_offset < clamped_size)
	{
		const int read_size = stream_->read(bytes + bytes_offset, clamped_size - bytes_offset);
		if (read_size <= 0)
		{
			break;
		}
		bytes_offset += read_size;
	}
	stream_position_ += bytes_offset;
	return static_cast<std::size_t>(bytes_offset);
}

bool FlacAudioDecoder::impl_callback_seek(int offset, drflac_seek_origin origin)
{
	std::int64_t position;
	switch (origin)
	{
		case DRFLAC_SEEK_SET:
			position = offset;
			break;
		case DRFLAC_SEEK_CUR:
			position = stream_position_ + offset;
			break;
		default:
			return false;
	}
	if (position < 0)
	{
		return false;
	}
	if (position >= stream_position_)
	{
		return impl_skip(position - stream_position_);
	}
	// The VFS stream only rewinds; walk forward from the start.
	if (!stream_->rewind())
	{
		return false;
	}
	stream_position_ = 0;
	return impl_skip(position);
}

bool FlacAudioDecoder::impl_skip(std::int64_t size)
{
	unsigned char skip_buffer[skip_buffer_size];
	while (size > 0)
	{
		const int to_read_size = static_cast<int>(std::min<std::int64_t>(size, skip_buffer_size));
		const int read_size = stream_->read(skip_buffer, to_read_size);
		if (read_size <= 0)
		{
			return false;
		}
		stream_position_ += read_size;
		size -= read_size;
	}
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
	flac_ = drflac_open(
		&FlacAudioDecoder::flac_callback_read,
		&FlacAudioDecoder::flac_callback_seek,
		&FlacAudioDecoder::flac_callback_tell,
		this,
		nullptr);
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
	stream_ = nullptr;
	stream_position_ = 0;
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
	stream_ = std::move(param.vfs_stream);
	stream_position_ = 0;
	if (!impl_open_flac())
	{
		return false;
	}
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
	{
		return true;
	}
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
		{
			break;
		}
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
			{
				break;
			}
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

