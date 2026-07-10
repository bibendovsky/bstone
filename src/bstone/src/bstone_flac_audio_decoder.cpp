#include "bstone_flac_audio_decoder.h"
#include "bstone_assert.h"
#include <algorithm>
#include <utility>
#include "FLAC/stream_decoder.h"

namespace bstone {

namespace {

class FlacAudioDecoder final : public AudioDecoder
{
public:
	FlacAudioDecoder();
	~FlacAudioDecoder() override;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int max_frames) override;
	bool rewind() override;

private:
	inline static constexpr int max_channels = 2;

	using CacheFrame = float[max_channels];
	using SampleConverter = float (*)(FLAC__int32 sample);
	using DecodeFramesFunc = int (FlacAudioDecoder::*)(float* samples, int max_frames);

	bool is_initialized_{};
	const char* error_message_{};
	FLAC__StreamDecoder* decoder_{};
	VfsInputStreamUPtr stream_{};
	int bit_depth_{};
	int channel_count_{};
	int src_frame_rate_{};
	int dst_frame_rate_{};
	SampleConverter sample_converter_{};
	const FLAC__int32* const* cache_samples_{};
	int cache_frame_count_{};
	int cache_frame_offset_{};
	int cache_frame_offset_counter_{};
	CacheFrame cache_frame_{};
	DecodeFramesFunc decode_frames_{};

private:
	bool is_error_message_empty() const;
	void clear_error_message();
	void set_error_message(const char* message);
	void set_error_message(FLAC__StreamDecoderState state);
	void set_error_message(FLAC__StreamDecoderInitStatus status);
	void set_error_message(FLAC__StreamDecoderReadStatus status);
	void set_error_message(FLAC__StreamDecoderSeekStatus status);
	void set_error_message(FLAC__StreamDecoderTellStatus status);
	void set_error_message(FLAC__StreamDecoderLengthStatus status);
	void set_error_message(FLAC__StreamDecoderErrorStatus status);

	static FLAC__StreamDecoderReadStatus read_callback_proxy(
		const FLAC__StreamDecoder* decoder,
		FLAC__byte buffer[],
		std::size_t* bytes,
		void* client_data);
	static FLAC__StreamDecoderWriteStatus write_callback_proxy(
		const FLAC__StreamDecoder* decoder,
		const FLAC__Frame* frame,
		const FLAC__int32* const buffer[],
		void* client_data);
	static void metadata_callback_proxy(
		const FLAC__StreamDecoder* decoder,
		const FLAC__StreamMetadata* metadata,
		void* client_data);
	static void error_callback_flac(
		const FLAC__StreamDecoder* decoder,
		FLAC__StreamDecoderErrorStatus status,
		void* client_data);

	FLAC__StreamDecoderReadStatus read_callback(FLAC__byte buffer[], std::size_t* bytes);
	FLAC__StreamDecoderWriteStatus write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[]);
	void metadata_callback(const FLAC__StreamMetadata* metadata);
	void error_callback(FLAC__StreamDecoderErrorStatus status);

	static float convert_sample_s8(FLAC__int32 sample);
	static float convert_sample_s16(FLAC__int32 sample);
	static float convert_sample_s24(FLAC__int32 sample);
	static float convert_sample_s32(FLAC__int32 sample);

	bool impl_is_initialized() const;
	void impl_close_flac();
	bool impl_open_flac();
	void impl_terminate();
	bool impl_initialize(const AudioDecoderInitParam& param);
	int decode_frames_as_is(float* samples, int max_frames);
	void cache_update_frame();
	bool update_cache();
	int decode_frames_with_resample(float* samples, int max_frames);
};

// -------------------------------------

FlacAudioDecoder::FlacAudioDecoder()
	:
	decoder_{FLAC__stream_decoder_new()}
{}

FlacAudioDecoder::~FlacAudioDecoder()
{
	FLAC__stream_decoder_delete(decoder_);
}

bool FlacAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	clear_error_message();
	if (decoder_ == nullptr)
	{
		set_error_message("Failed to allocate a decoder.");
		return false;
	}
	impl_terminate();
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
	BSTONE_ASSERT(impl_is_initialized());
	BSTONE_ASSERT(max_frames >= 0);
	BSTONE_ASSERT(decode_frames_ != nullptr);
	return (this->*decode_frames_)(samples, max_frames);
}

bool FlacAudioDecoder::rewind()
{
	BSTONE_ASSERT(impl_is_initialized());
	impl_close_flac();
	if (!stream_->rewind())
	{
		set_error_message("Failed to rewind a stream.");
		return false;
	}
	const int old_bit_depth_ = bit_depth_;
	const int old_channel_count_ = channel_count_;
	const int old_src_sample_rate_ = src_frame_rate_;
	if (!impl_open_flac())
		return false;
	if (bit_depth_ != old_bit_depth_ ||
		channel_count_ != old_channel_count_ ||
		src_frame_rate_ != old_src_sample_rate_)
	{
		set_error_message("Parameters mismatch.");
		return false;
	}
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	return true;
}

bool FlacAudioDecoder::is_error_message_empty() const
{
	return error_message_ == nullptr || error_message_[0] == '\0';
}

void FlacAudioDecoder::clear_error_message()
{
	error_message_ = nullptr;
}

void FlacAudioDecoder::set_error_message(const char* message)
{
	error_message_ = message;
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderState state)
{
	set_error_message(FLAC__StreamDecoderStateString[state]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderInitStatus status)
{
	set_error_message(FLAC__StreamDecoderInitStatusString[status]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderReadStatus status)
{
	set_error_message(FLAC__StreamDecoderReadStatusString[status]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderSeekStatus status)
{
	set_error_message(FLAC__StreamDecoderSeekStatusString[status]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderTellStatus status)
{
	set_error_message(FLAC__StreamDecoderTellStatusString[status]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderLengthStatus status)
{
	set_error_message(FLAC__StreamDecoderLengthStatusString[status]);
}

void FlacAudioDecoder::set_error_message(FLAC__StreamDecoderErrorStatus status)
{
	set_error_message(FLAC__StreamDecoderErrorStatusString[status]);
}

FLAC__StreamDecoderReadStatus FlacAudioDecoder::read_callback_proxy(
	[[maybe_unused]] const FLAC__StreamDecoder* decoder,
	FLAC__byte buffer[],
	std::size_t* bytes,
	void* client_data)
{
	return static_cast<FlacAudioDecoder*>(client_data)->read_callback(buffer, bytes);
}

FLAC__StreamDecoderWriteStatus FlacAudioDecoder::write_callback_proxy(
	[[maybe_unused]] const FLAC__StreamDecoder* decoder,
	const FLAC__Frame* frame,
	const FLAC__int32* const buffer[],
	void* client_data)
{
	return static_cast<FlacAudioDecoder*>(client_data)->write_callback(frame, buffer);
}

void FlacAudioDecoder::metadata_callback_proxy(
	[[maybe_unused]] const FLAC__StreamDecoder* decoder,
	const FLAC__StreamMetadata* metadata,
	void* client_data)
{
	static_cast<FlacAudioDecoder*>(client_data)->metadata_callback(metadata);
}

void FlacAudioDecoder::error_callback_flac(
	[[maybe_unused]] const FLAC__StreamDecoder* decoder,
	FLAC__StreamDecoderErrorStatus status,
	void* client_data)
{
	static_cast<FlacAudioDecoder*>(client_data)->error_callback(status);
}

FLAC__StreamDecoderReadStatus FlacAudioDecoder::read_callback(FLAC__byte buffer[], std::size_t* bytes)
{
	const int to_read_size = static_cast<int>(*bytes);
	const int read_size = stream_->read(buffer, to_read_size);
	*bytes = 0;
	if (read_size < 0)
		return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
	if (read_size == 0 && to_read_size > 0)
		return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
	*bytes = static_cast<std::size_t>(read_size);
	return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus FlacAudioDecoder::write_callback(const FLAC__Frame* frame, const FLAC__int32* const buffer[])
{
	cache_samples_ = buffer;
	cache_frame_count_ = static_cast<int>(frame->header.blocksize);
	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

void FlacAudioDecoder::metadata_callback(const FLAC__StreamMetadata* metadata)
{
	if (metadata->type != FLAC__METADATA_TYPE_STREAMINFO)
		return;
	const FLAC__StreamMetadata_StreamInfo& info = metadata->data.stream_info;
	bit_depth_ = static_cast<int>(info.bits_per_sample);
	channel_count_ = static_cast<int>(info.channels);
	src_frame_rate_ = static_cast<int>(info.sample_rate);
}

void FlacAudioDecoder::error_callback(FLAC__StreamDecoderErrorStatus status)
{
	set_error_message(FLAC__StreamDecoderErrorStatusString[status]);
}

float FlacAudioDecoder::convert_sample_s8(FLAC__int32 sample)
{
	return static_cast<float>(sample) / 128.0F;
}

float FlacAudioDecoder::convert_sample_s16(FLAC__int32 sample)
{
	return static_cast<float>(sample) / 32768.0F;
}

float FlacAudioDecoder::convert_sample_s24(FLAC__int32 sample)
{
	return static_cast<float>(sample) / 8388608.0F;
}

float FlacAudioDecoder::convert_sample_s32(FLAC__int32 sample)
{
	return static_cast<float>(sample) / 2147483648.0F;
}

bool FlacAudioDecoder::impl_is_initialized() const
{
	return is_initialized_;
}

void FlacAudioDecoder::impl_close_flac()
{
	FLAC__stream_decoder_finish(decoder_);
}

bool FlacAudioDecoder::impl_open_flac()
{
	if (!FLAC__stream_decoder_get_md5_checking(decoder_))
	{
		if (!FLAC__stream_decoder_set_md5_checking(decoder_, false))
		{
			set_error_message("Failed to disable MD5 checking.");
			return false;
		}
	}
	const FLAC__StreamDecoderInitStatus init_status = FLAC__stream_decoder_init_stream(
		/* decoder           */ decoder_,
		/* read_callback     */ &FlacAudioDecoder::read_callback_proxy,
		/* seek_callback     */ nullptr,
		/* tell_callback     */ nullptr,
		/* length_callback   */ nullptr,
		/* eof_callback      */ nullptr,
		/* write_callback    */ &FlacAudioDecoder::write_callback_proxy,
		/* metadata_callback */ &FlacAudioDecoder::metadata_callback_proxy,
		/* error_callback    */ &FlacAudioDecoder::error_callback_flac,
		/* client_data       */ this);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK)
	{
		set_error_message(init_status);
		return false;
	}
	clear_error_message();
	if (!FLAC__stream_decoder_process_until_end_of_metadata(decoder_))
	{
		if (is_error_message_empty())
			set_error_message("Failed to process all metadata.");
		return false;
	}
	return true;
}

void FlacAudioDecoder::impl_terminate()
{
	impl_close_flac();
	is_initialized_ = false;
	stream_ = nullptr;
}

bool FlacAudioDecoder::impl_initialize(const AudioDecoderInitParam& param)
{
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
	dst_frame_rate_ = param.dst_rate;
	if (!impl_open_flac())
		return false;
	switch (bit_depth_)
	{
		case 8:
			sample_converter_ = &FlacAudioDecoder::convert_sample_s8;
			break;
		case 16:
			sample_converter_ = &FlacAudioDecoder::convert_sample_s16;
			break;
		case 24:
			sample_converter_ = &FlacAudioDecoder::convert_sample_s24;
			break;
		case 32:
			sample_converter_ = &FlacAudioDecoder::convert_sample_s32;
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
	cache_samples_ = nullptr;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	std::fill_n(cache_frame_, channel_count_, 0.0F);
	if (src_frame_rate_ == dst_frame_rate_)
		decode_frames_ = &FlacAudioDecoder::decode_frames_as_is;
	else
		decode_frames_ = &FlacAudioDecoder::decode_frames_with_resample;
	is_initialized_ = true;
	return true;
}

int FlacAudioDecoder::decode_frames_as_is(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count != max_frames)
	{
		if (cache_frame_offset_ >= cache_frame_count_)
		{
			cache_frame_count_ = 0;
			cache_frame_offset_ = 0;
			if (!FLAC__stream_decoder_process_single(decoder_))
				return -1;
			if (cache_frame_count_ == 0)
				break;
		}
		const int frame_count = std::min(max_frames - written_frame_count, cache_frame_count_ - cache_frame_offset_);
		for (int i_frame = 0; i_frame < frame_count; ++i_frame)
		{
			for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
			{
				const FLAC__int32 sample_s32 = cache_samples_[i_channel][cache_frame_offset_ + i_frame];
				samples[i_channel] = sample_converter_(sample_s32);
			}
			samples += channel_count_;
		}
		written_frame_count += frame_count;
		cache_frame_offset_ += frame_count;
	}
	return written_frame_count;
}

void FlacAudioDecoder::cache_update_frame()
{
	for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
	{
		const FLAC__int32 sample_s32 = cache_samples_[i_channel][cache_frame_offset_];
		cache_frame_[i_channel] = sample_converter_(sample_s32);
	}
}

bool FlacAudioDecoder::update_cache()
{
	if (cache_frame_offset_ < cache_frame_count_)
		return true;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	if (!FLAC__stream_decoder_process_single(decoder_))
		return false;
	if (cache_frame_offset_ >= cache_frame_count_)
		return false;
	cache_update_frame();
	return true;
}

int FlacAudioDecoder::decode_frames_with_resample(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count != max_frames)
	{
		if (!update_cache())
			break;
		std::copy_n(cache_frame_, channel_count_, samples);
		samples += channel_count_;
		++written_frame_count;
		cache_frame_offset_counter_ += src_frame_rate_;
		while (cache_frame_offset_counter_ >= dst_frame_rate_)
		{
			cache_frame_offset_counter_ -= dst_frame_rate_;
			++cache_frame_offset_;
			if (!update_cache())
				break;
			cache_update_frame();
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
