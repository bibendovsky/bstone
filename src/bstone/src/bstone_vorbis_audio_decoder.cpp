#include "bstone_vorbis_audio_decoder.h"
#include "bstone_assert.h"
#include <cstddef>
#include <algorithm>
#include <limits>
#include <utility>
#include "vorbis/vorbisfile.h"

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
	int decode_frames(float* samples, int sample_count) override;
	bool rewind() override;

private:
	inline static constexpr int cache_capacity = 256;

	using DecodeFramesFunc = int (VorbisAudioDecoder::*)(float* samples, int frame_count);

	const char* error_message_{};
	OggVorbis_File vf_{};
	VfsInputStreamUPtr stream_{};
	int channel_count_{};
	int src_frame_rate_{};
	int dst_frame_rate_{};
	float** cache_samples_{};
	int cache_frame_count_{};
	int cache_frame_offset_{};
	int cache_frame_offset_counter_{};
	DecodeFramesFunc decode_frames_{};

	const char* get_vorbis_error_code_string(int error_code);
	void set_error_message(const char* message);
	void set_error_message_from_vorbis_code(int error_code);

	static std::size_t vorbis_callback_read(void* ptr, std::size_t size, std::size_t nmemb, void* datasource);

	bool impl_is_initialized() const;
	void impl_terminate_vorbis();
	bool impl_initialize_vorbis();
	void impl_terminate();
	bool impl_initialize(const AudioDecoderInitParam& param);

	std::size_t impl_callback_read(void* ptr, std::size_t size, std::size_t nmemb);
	int decode_frames_as_is(float* samples, int max_frames);
	bool update_cache();
	int decode_frames_with_resample(float* samples, int max_frames);
};

// -------------------------------------

VorbisAudioDecoder::~VorbisAudioDecoder()
{
	impl_terminate_vorbis();
}

bool VorbisAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
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
	if (!stream_->rewind())
	{
		set_error_message("Failed to rewind a stream.");
		return false;
	}
	const int old_channel_count = channel_count_;
	const int old_src_sample_rate = src_frame_rate_;
	if (!impl_initialize_vorbis())
		return false;
	if (channel_count_ != old_channel_count ||
		src_frame_rate_ != old_src_sample_rate)
	{
		set_error_message("Parameters mismatch.");
		return false;
	}
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	return true;
}

const char* VorbisAudioDecoder::get_vorbis_error_code_string(int error_code)
{
	BSTONE_ASSERT(error_code < 0);
#define BSTONE_MACRO(x) case x: return #x;
	switch (error_code)
	{
		BSTONE_MACRO(OV_FALSE);
		BSTONE_MACRO(OV_EOF);
		BSTONE_MACRO(OV_HOLE);
		BSTONE_MACRO(OV_EREAD);
		BSTONE_MACRO(OV_EFAULT);
		BSTONE_MACRO(OV_EIMPL);
		BSTONE_MACRO(OV_EINVAL);
		BSTONE_MACRO(OV_ENOTVORBIS);
		BSTONE_MACRO(OV_EBADHEADER);
		BSTONE_MACRO(OV_EVERSION);
		BSTONE_MACRO(OV_ENOTAUDIO);
		BSTONE_MACRO(OV_EBADPACKET);
		BSTONE_MACRO(OV_EBADLINK);
		BSTONE_MACRO(OV_ENOSEEK);
		default: return "OV_???";
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

std::size_t VorbisAudioDecoder::vorbis_callback_read(void* ptr, std::size_t size, std::size_t nmemb, void* datasource)
{
	return static_cast<VorbisAudioDecoder*>(datasource)->impl_callback_read(ptr, size, nmemb);
}

bool VorbisAudioDecoder::impl_is_initialized() const
{
	return stream_ != nullptr;
}

void VorbisAudioDecoder::impl_terminate_vorbis()
{
	ov_clear(&vf_);
}

bool VorbisAudioDecoder::impl_initialize_vorbis()
{
	ov_callbacks callbacks{
		.read_func = &VorbisAudioDecoder::vorbis_callback_read,
		.seek_func = nullptr,
		.close_func = nullptr,
		.tell_func = nullptr};
	const int ov_open_result = ov_open_callbacks(this, &vf_, nullptr, 0, callbacks);
	if (ov_open_result != 0)
	{
		set_error_message_from_vorbis_code(ov_open_result);
		return false;
	}
	const vorbis_info* const info = ov_info(&vf_, -1);
	if (info == nullptr)
	{
		set_error_message("Failed to get Vorbis info.");
		return false;
	}
	if (ov_streams(&vf_) != 1)
	{
		set_error_message("Multiple logical bitstreams.");
		return false;
	}
	channel_count_ = info->channels;
	src_frame_rate_ = static_cast<int>(info->rate);
	return true;
}

void VorbisAudioDecoder::impl_terminate()
{
	stream_ = nullptr;
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
	if (!impl_initialize_vorbis())
		return false;
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
		decode_frames_ = &VorbisAudioDecoder::decode_frames_as_is;
	else
		decode_frames_ = &VorbisAudioDecoder::decode_frames_with_resample;
	return true;
}

std::size_t VorbisAudioDecoder::impl_callback_read(void* ptr, std::size_t size, std::size_t nmemb)
{
	if (const int read_size = stream_->read(ptr, static_cast<int>(size * nmemb));
		read_size > 0)
		return read_size;
	return 0;
}

int VorbisAudioDecoder::decode_frames_as_is(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		int bitstream;
		float** vorbis_samples;
		const long read_frame_count = ov_read_float(&vf_, &vorbis_samples, max_frames - written_frame_count, &bitstream);
		if (read_frame_count == 0)
			break;
		if (read_frame_count < 0)
		{
			if (read_frame_count == OV_HOLE)
				continue;
			set_error_message_from_vorbis_code(read_frame_count);
			return -1;
		}
		for (int i_frame = 0; i_frame < read_frame_count; ++i_frame)
		{
			for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
				*samples++ = vorbis_samples[i_channel][i_frame];
		}
		written_frame_count += read_frame_count;
	}
	return written_frame_count;
}

bool VorbisAudioDecoder::update_cache()
{
	if (cache_frame_offset_ < cache_frame_count_)
		return true;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	for (;;)
	{
		const long read_frame_count = ov_read_float(&vf_, &cache_samples_, cache_capacity, nullptr);
		if (read_frame_count < 0)
		{
			if (read_frame_count == OV_HOLE)
				continue;
			return false;
		}
		cache_frame_count_ = static_cast<int>(read_frame_count);
		return cache_frame_offset_ < cache_frame_count_;
	}
}

int VorbisAudioDecoder::decode_frames_with_resample(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		if (!update_cache())
			break;
		for (int i_channel = 0; i_channel < channel_count_; ++i_channel)
			samples[i_channel] = cache_samples_[i_channel][cache_frame_offset_];
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

AudioDecoderUPtr make_vorbis_audio_decoder()
{
	return std::make_unique<VorbisAudioDecoder>();
}

} // namespace bstone
