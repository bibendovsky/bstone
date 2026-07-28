#include "bstone_vorbis_audio_decoder.h"
#include "bstone_assert.h"
#include <cstddef>
#include <cstring>
#include <algorithm>
#include <utility>
#include <vector>
#include "stb_vorbis.h"

namespace bstone {

namespace {

class VorbisAudioDecoder final : public AudioDecoder
{
public:
	VorbisAudioDecoder() = default;
	~VorbisAudioDecoder() override;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int max_frames) override;
	bool rewind() override;

private:
	// An Ogg page rarely exceeds 8 KiB; the window doubles if one still does not fit.
	inline static constexpr int initial_buffer_capacity = 32 * 1024;
	inline static constexpr int max_buffer_capacity = 1024 * 1024;

	using DecodeFramesFunc = int (VorbisAudioDecoder::*)(float* samples, int max_frames);

	const char* error_message_{};
	stb_vorbis* vorbis_{};
	VfsInputStreamUPtr stream_{};
	// A bounded window over the stream; stb_vorbis consumes it packet by packet.
	std::vector<unsigned char> buffer_{};
	int buffer_position_{};
	int buffer_end_{};
	bool is_stream_at_end_{};
	bool is_open_{};
	int channel_count_{};
	int src_frame_rate_{};
	int dst_frame_rate_{};
	// Points into stb_vorbis's internal buffer; valid until the next decode call.
	float** cache_samples_{};
	int cache_frame_count_{};
	int cache_frame_offset_{};
	int cache_frame_offset_counter_{};
	DecodeFramesFunc decode_frames_{};

	static const char* get_vorbis_error_code_string(int error_code);
	void set_error_message(const char* message);
	void set_error_message_from_vorbis_code(int error_code);

	bool impl_is_initialized() const;
	void impl_close_vorbis();
	bool refill_buffer();
	bool grow_buffer();
	bool impl_open_vorbis();
	void impl_terminate();
	bool impl_initialize(const AudioDecoderInitParam& param);
	bool refill_cache();
	bool update_cache();
	int decode_frames_as_is(float* samples, int max_frames);
	int decode_frames_with_resample(float* samples, int max_frames);
};

// -------------------------------------

VorbisAudioDecoder::~VorbisAudioDecoder()
{
	impl_close_vorbis();
}

bool VorbisAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	error_message_ = nullptr;
	if (!impl_initialize(param))
	{
		impl_terminate();
		return false;
	}
	return true;
}

void VorbisAudioDecoder::terminate()
{
	impl_terminate();
}

bool VorbisAudioDecoder::is_initialized() const
{
	return impl_is_initialized();
}

const char* VorbisAudioDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int VorbisAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(impl_is_initialized());
	return channel_count_;
}

int VorbisAudioDecoder::decode_frames(float* samples, int max_frames)
{
	BSTONE_ASSERT(max_frames >= 0);
	BSTONE_ASSERT(impl_is_initialized());
	return (this->*decode_frames_)(samples, max_frames);
}

bool VorbisAudioDecoder::rewind()
{
	BSTONE_ASSERT(impl_is_initialized());
	// Push mode cannot seek; reopen from the rewound stream.
	if (!stream_->rewind())
	{
		set_error_message("Failed to rewind a stream.");
		return false;
	}
	impl_close_vorbis();
	buffer_position_ = 0;
	buffer_end_ = 0;
	is_stream_at_end_ = false;
	if (!refill_buffer())
	{
		set_error_message("Empty or invalid stream.");
		return false;
	}
	if (!impl_open_vorbis())
	{
		return false;
	}
	cache_samples_ = nullptr;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	return true;
}

const char* VorbisAudioDecoder::get_vorbis_error_code_string(int error_code)
{
#define BSTONE_MACRO(x) case x: return #x;
	switch (error_code)
	{
		BSTONE_MACRO(VORBIS__no_error);
		BSTONE_MACRO(VORBIS_need_more_data);
		BSTONE_MACRO(VORBIS_invalid_api_mixing);
		BSTONE_MACRO(VORBIS_outofmem);
		BSTONE_MACRO(VORBIS_feature_not_supported);
		BSTONE_MACRO(VORBIS_too_many_channels);
		BSTONE_MACRO(VORBIS_file_open_failure);
		BSTONE_MACRO(VORBIS_seek_without_length);
		BSTONE_MACRO(VORBIS_unexpected_eof);
		BSTONE_MACRO(VORBIS_seek_invalid);
		BSTONE_MACRO(VORBIS_invalid_setup);
		BSTONE_MACRO(VORBIS_invalid_stream);
		BSTONE_MACRO(VORBIS_missing_capture_pattern);
		BSTONE_MACRO(VORBIS_invalid_stream_structure_version);
		BSTONE_MACRO(VORBIS_continued_packet_flag_invalid);
		BSTONE_MACRO(VORBIS_incorrect_stream_serial_number);
		BSTONE_MACRO(VORBIS_invalid_first_page);
		BSTONE_MACRO(VORBIS_bad_packet_type);
		BSTONE_MACRO(VORBIS_cant_find_last_page);
		BSTONE_MACRO(VORBIS_seek_failed);
		BSTONE_MACRO(VORBIS_ogg_skeleton_not_supported);
		default: return "VORBIS_???";
	}
#undef BSTONE_MACRO
}

void VorbisAudioDecoder::set_error_message(const char* message)
{
	error_message_ = message;
}

void VorbisAudioDecoder::set_error_message_from_vorbis_code(int error_code)
{
	set_error_message(get_vorbis_error_code_string(error_code));
}

bool VorbisAudioDecoder::impl_is_initialized() const
{
	return is_open_;
}

void VorbisAudioDecoder::impl_close_vorbis()
{
	if (vorbis_ != nullptr)
	{
		stb_vorbis_close(vorbis_);
		vorbis_ = nullptr;
	}
	is_open_ = false;
}

bool VorbisAudioDecoder::refill_buffer()
{
	if (buffer_position_ > 0)
	{
		std::memmove(buffer_.data(), buffer_.data() + buffer_position_, buffer_end_ - buffer_position_);
		buffer_end_ -= buffer_position_;
		buffer_position_ = 0;
	}
	if (is_stream_at_end_)
	{
		return false;
	}
	const int read_size = stream_->read(buffer_.data() + buffer_end_, static_cast<int>(buffer_.size()) - buffer_end_);
	if (read_size <= 0)
	{
		is_stream_at_end_ = true;
		return false;
	}
	buffer_end_ += read_size;
	return true;
}

bool VorbisAudioDecoder::grow_buffer()
{
	if (buffer_.size() >= max_buffer_capacity)
	{
		set_error_message("Ogg page too big.");
		return false;
	}
	buffer_.resize(buffer_.size() * 2);
	return true;
}

bool VorbisAudioDecoder::impl_open_vorbis()
{
	while (true)
	{
		int consumed_size = 0;
		int error_code = VORBIS__no_error;
		vorbis_ = stb_vorbis_open_pushdata(
			buffer_.data() + buffer_position_,
			buffer_end_ - buffer_position_,
			&consumed_size,
			&error_code,
			nullptr);
		if (vorbis_ != nullptr)
		{
			buffer_position_ += consumed_size;
			break;
		}
		if (error_code != VORBIS_need_more_data)
		{
			set_error_message_from_vorbis_code(error_code);
			return false;
		}
		if (buffer_end_ - buffer_position_ == static_cast<int>(buffer_.size()) && !grow_buffer())
		{
			return false;
		}
		if (!refill_buffer())
		{
			set_error_message_from_vorbis_code(VORBIS_unexpected_eof);
			return false;
		}
	}
	const stb_vorbis_info info = stb_vorbis_get_info(vorbis_);
	channel_count_ = info.channels;
	src_frame_rate_ = static_cast<int>(info.sample_rate);
	is_open_ = true;
	return true;
}

void VorbisAudioDecoder::impl_terminate()
{
	impl_close_vorbis();
	stream_ = nullptr;
	buffer_.clear();
	buffer_.shrink_to_fit();
	buffer_position_ = 0;
	buffer_end_ = 0;
	is_stream_at_end_ = false;
}

bool VorbisAudioDecoder::impl_initialize(const AudioDecoderInitParam& param)
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
	buffer_.resize(initial_buffer_capacity);
	if (!refill_buffer())
	{
		set_error_message("Empty or invalid stream.");
		return false;
	}
	if (!impl_open_vorbis())
	{
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
	cache_samples_ = nullptr;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	if (src_frame_rate_ == dst_frame_rate_)
	{
		decode_frames_ = &VorbisAudioDecoder::decode_frames_as_is;
	}
	else
	{
		decode_frames_ = &VorbisAudioDecoder::decode_frames_with_resample;
	}
	return true;
}

bool VorbisAudioDecoder::refill_cache()
{
	while (true)
	{
		int channels = 0;
		float** outputs = nullptr;
		int frame_count = 0;
		const int consumed_size = stb_vorbis_decode_frame_pushdata(
			vorbis_,
			buffer_.data() + buffer_position_,
			buffer_end_ - buffer_position_,
			&channels,
			&outputs,
			&frame_count);
		if (consumed_size > 0)
		{
			buffer_position_ += consumed_size;
			if (frame_count > 0)
			{
				cache_samples_ = outputs;
				cache_frame_count_ = frame_count;
				cache_frame_offset_ = 0;
				return true;
			}
			// A consumed packet with no frames: resynchronisation or a header.
			continue;
		}
		if (buffer_end_ - buffer_position_ == static_cast<int>(buffer_.size()) && !grow_buffer())
		{
			return false;
		}
		if (!refill_buffer())
		{
			return false;
		}
	}
}

bool VorbisAudioDecoder::update_cache()
{
	if (cache_frame_offset_ < cache_frame_count_)
	{
		return true;
	}
	return refill_cache();
}

int VorbisAudioDecoder::decode_frames_as_is(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		if (cache_frame_offset_ >= cache_frame_count_)
		{
			if (!refill_cache())
			{
				break;
			}
		}
		const int frame_count = std::min(max_frames - written_frame_count, cache_frame_count_ - cache_frame_offset_);
		for (int i_frame = 0; i_frame < frame_count; ++i_frame)
		{
			for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
			{
				samples[i_channel] = cache_samples_[i_channel][cache_frame_offset_ + i_frame];
			}
			samples += channel_count_;
		}
		written_frame_count += frame_count;
		cache_frame_offset_ += frame_count;
	}
	return written_frame_count;
}

int VorbisAudioDecoder::decode_frames_with_resample(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		if (!update_cache())
		{
			break;
		}
		for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
		{
			samples[i_channel] = cache_samples_[i_channel][cache_frame_offset_];
		}
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

AudioDecoderUPtr make_vorbis_audio_decoder()
{
	return std::make_unique<VorbisAudioDecoder>();
}

} // namespace bstone
