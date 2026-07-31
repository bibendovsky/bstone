#include "bstone_wav_audio_decoder.h"
#include "bstone_assert.h"
#include "bstone_endian.h"
#include "bstone_memory_binary_reader.h"
#include <algorithm>
#include <memory>

namespace bstone {

namespace {

class WavAudioDecoder final : public AudioDecoder
{
public:
	WavAudioDecoder() = default;
	~WavAudioDecoder() override = default;

	bool initialize(const AudioDecoderInitParam& param) override;
	void terminate() override;
	bool is_initialized() const override;
	const char* get_error_message() const override;
	int get_channel_count() const override;
	int decode_frames(float* samples, int max_frames) override;
	bool rewind() override;

private:
	inline static constexpr int max_channels = 2;
	inline static constexpr int riff_header_size = 12;
	inline static constexpr int chunk_header_size = 8;
	inline static constexpr int fmt0x20_min_chunk_size = 16;
	inline static constexpr int skip_buffer_size = 256;
	inline static constexpr int byte_cache_capacity = 1024;
	inline static constexpr int cache_capacity = byte_cache_capacity / (max_channels * static_cast<int>(sizeof(float)));

	using Cache = float[cache_capacity * max_channels];
	using ConvertSamplesFunc = void (WavAudioDecoder::*)(const unsigned char* bytes, int sample_count);
	using DecodeFramesFunc = int (WavAudioDecoder::*)(float* samples, int frame_count);

	bool is_initialized_{};
	const char* error_message_{};
	VfsInputStreamUPtr stream_{};
	int channel_count_{};
	int src_byte_depth_{};
	int src_sample_rate_{};
	int dst_sample_rate_{};
	int cache_frame_count_{};
	int cache_frame_offset_{};
	int cache_frame_offset_counter_{};
	int wav_size_{};
	int wav_offset_{};
	int data_offset_{};
	ConvertSamplesFunc convert_samples_{};
	Cache cache_{};
	DecodeFramesFunc decode_frames_{};

	void set_error_message(const char* message);

	bool impl_is_initialized() const;
	void impl_wav_close();
	bool impl_wav_open();
	void impl_terminate();
	bool impl_initialize(const AudioDecoderInitParam& param);
	bool impl_wav_read(void* buffer, int size);
	bool impl_wav_skip(int size);
	bool impl_wav_read_fmt0x20();

	void convert_u8(const unsigned char* bytes, int sample_count);
	void convert_s16(const unsigned char* bytes, int sample_count);
	void convert_s24(const unsigned char* bytes, int sample_count);
	void convert_s32(const unsigned char* bytes, int sample_count);
	void convert_f32_iec559(const unsigned char* bytes, int sample_count);
	int decode_frames_as_is(float* samples, int max_frames);
	bool update_cache();
	int decode_frames_with_resample(float* samples, int max_frames);
};

// -------------------------------------

bool WavAudioDecoder::initialize(const AudioDecoderInitParam& param)
{
	impl_terminate();
	if (!impl_initialize(param))
	{
		impl_terminate();
		return false;
	}
	return true;
}

void WavAudioDecoder::terminate()
{
	impl_terminate();
}

bool WavAudioDecoder::is_initialized() const
{
	return impl_is_initialized();
}

const char* WavAudioDecoder::get_error_message() const
{
	return error_message_ != nullptr ? error_message_ : "";
}

int WavAudioDecoder::get_channel_count() const
{
	BSTONE_ASSERT(impl_is_initialized());
	return channel_count_;
}

int WavAudioDecoder::decode_frames(float* samples, int max_frames)
{
	BSTONE_ASSERT(impl_is_initialized());
	BSTONE_ASSERT(max_frames >= 0);
	return (this->*decode_frames_)(samples, max_frames);
}

bool WavAudioDecoder::rewind()
{
	BSTONE_ASSERT(impl_is_initialized());
	if (!stream_->rewind())
	{
		set_error_message("Failed to reset a stream.");
		return false;
	}
	if (!impl_wav_skip(data_offset_))
	{
		return false;
	}
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	wav_offset_ = 0;
	return true;
}

void WavAudioDecoder::set_error_message(const char* message)
{
	error_message_ = message;
}

bool WavAudioDecoder::impl_is_initialized() const
{
	return is_initialized_;
}

void WavAudioDecoder::impl_wav_close()
{
	wav_size_ = 0;
	wav_offset_ = 0;
	data_offset_ = 0;
}

bool WavAudioDecoder::impl_wav_open()
{
	constexpr unsigned int wav_max_chunk_size = 0x7FFFFFFFU - chunk_header_size;
	unsigned char riff_header[riff_header_size];
	if (!impl_wav_read(riff_header, riff_header_size))
	{
		return false;
	}
	MemoryBinaryReader riff_reader{riff_header, riff_header_size};
	// RIFF id
	if (riff_reader.read_u32_le() != 0x46464952U)
	{
		set_error_message("Expected 'RIFF' chunk id.");
		return false;
	}
	// RIFF chunk size
	const unsigned int riff_chunk_size_u32 = riff_reader.read_u32_le();
	if (riff_chunk_size_u32 > wav_max_chunk_size)
	{
		set_error_message("'RIFF' chunk size too big.");
		return false;
	}
	// WAVE id
	if (riff_reader.read_u32_le() != 0x45564157U)
	{
		set_error_message("Expected 'WAVE' chunk id.");
		return false;
	}
	// Walk the chunks until "data" turns up. Nothing in the format promises
	// that "fmt " and "data" are adjacent, or that "fmt " has no extension -
	// encoders routinely interleave "fact", "LIST" or padding chunks.
	const int riff_end_offset = static_cast<int>(riff_chunk_size_u32) + chunk_header_size;
	bool has_fmt0x20 = false;
	int offset = riff_header_size;
	for (;;)
	{
		if (riff_end_offset - offset < chunk_header_size)
		{
			set_error_message("Missing 'data' chunk.");
			return false;
		}
		unsigned char chunk_header[chunk_header_size];
		if (!impl_wav_read(chunk_header, chunk_header_size))
		{
			return false;
		}
		MemoryBinaryReader chunk_reader{chunk_header, chunk_header_size};
		const unsigned int chunk_id_u32 = chunk_reader.read_u32_le();
		const unsigned int chunk_size_u32 = chunk_reader.read_u32_le();
		offset += chunk_header_size;
		if (chunk_size_u32 > static_cast<unsigned int>(riff_end_offset - offset))
		{
			set_error_message("Chunk is beyond 'RIFF' bounds.");
			return false;
		}
		const int chunk_size = static_cast<int>(chunk_size_u32);
		switch (chunk_id_u32)
		{
			case 0x20746D66U: // "fmt "
				if (has_fmt0x20)
				{
					set_error_message("Duplicate 'fmt ' chunk.");
					return false;
				}
				if (chunk_size < fmt0x20_min_chunk_size)
				{
					set_error_message("Unsupported 'fmt ' chunk size.");
					return false;
				}
				if (!impl_wav_read_fmt0x20())
				{
					return false;
				}
				if (!impl_wav_skip(chunk_size - fmt0x20_min_chunk_size))
				{
					return false;
				}
				has_fmt0x20 = true;
				break;
			case 0x61746164U: // "data"
				if (!has_fmt0x20)
				{
					set_error_message("Missing 'fmt ' chunk.");
					return false;
				}
				wav_size_ = chunk_size;
				data_offset_ = offset;
				return true;
			default:
				if (!impl_wav_skip(chunk_size))
				{
					return false;
				}
				break;
		}
		offset += chunk_size;
		// Chunks are word-aligned, an odd-sized one is followed by a pad octet.
		if ((chunk_size % 2) != 0)
		{
			if (!impl_wav_skip(1))
			{
				return false;
			}
			++offset;
		}
	}
}

void WavAudioDecoder::impl_terminate()
{
	is_initialized_ = false;
	stream_ = nullptr;
}

bool WavAudioDecoder::impl_initialize(const AudioDecoderInitParam& param)
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
	if (!impl_wav_open())
	{
		return false;
	}
	dst_sample_rate_ = param.dst_rate;
	cache_frame_count_ = 0;
	cache_frame_offset_ = 0;
	cache_frame_offset_counter_ = 0;
	if (src_sample_rate_ == param.dst_rate)
	{
		decode_frames_ = &WavAudioDecoder::decode_frames_as_is;
	}
	else
	{
		decode_frames_ = &WavAudioDecoder::decode_frames_with_resample;
	}
	is_initialized_ = true;
	return true;
}

bool WavAudioDecoder::impl_wav_read(void* buffer, int size)
{
	if (!stream_->read_exactly(buffer, size))
	{
		set_error_message("Failed to read WAV meta data.");
		return false;
	}
	return true;
}

bool WavAudioDecoder::impl_wav_skip(int size)
{
	BSTONE_ASSERT(size >= 0);
	// The stream can only be read forward, so discard the bytes to skip over.
	unsigned char buffer[skip_buffer_size];
	int skipped_size = 0;
	while (skipped_size < size)
	{
		const int read_size = std::min(size - skipped_size, skip_buffer_size);
		if (!impl_wav_read(buffer, read_size))
		{
			return false;
		}
		skipped_size += read_size;
	}
	return true;
}

bool WavAudioDecoder::impl_wav_read_fmt0x20()
{
	constexpr int wav_format_pcm = 1;
	constexpr int wav_format_ieee_float = 3;
	unsigned char fmt0x20[fmt0x20_min_chunk_size];
	if (!impl_wav_read(fmt0x20, fmt0x20_min_chunk_size))
	{
		return false;
	}
	MemoryBinaryReader reader{fmt0x20, fmt0x20_min_chunk_size};
	// format tag
	const int format_tag = reader.read_u16_le();
	switch (format_tag)
	{
		case wav_format_pcm:
		case wav_format_ieee_float:
			break;
		default:
			set_error_message("Unsupported format.");
			return false;
	}
	// channel count
	const int channel_count = reader.read_u16_le();
	switch (channel_count)
	{
		case 1:
		case 2:
			break;
		default:
			set_error_message("Unsupported channel count.");
			return false;
	}
	// sample rate
	const unsigned int sample_rate_u32 = reader.read_u32_le();
	if (sample_rate_u32 < audio_decoder_min_sample_rate || sample_rate_u32 > audio_decoder_max_sample_rate)
	{
		set_error_message("Unsupported sample rate.");
		return false;
	}
	// byte rate
	const unsigned int byte_rate_u32 = reader.read_u32_le();
	// block align
	const int block_align = reader.read_u16_le();
	// bit depth
	const int bit_depth = reader.read_u16_le();
	switch (bit_depth)
	{
		case 8:
			convert_samples_ = &WavAudioDecoder::convert_u8;
			break;
		case 16:
			convert_samples_ = &WavAudioDecoder::convert_s16;
			break;
		case 24:
			convert_samples_ = &WavAudioDecoder::convert_s24;
			break;
		case 32:
			if (format_tag == wav_format_ieee_float)
				convert_samples_ = &WavAudioDecoder::convert_f32_iec559;
			else
				convert_samples_ = &WavAudioDecoder::convert_s32;
			break;
		default:
			set_error_message("Unsupported bit depth.");
			return false;
	}
	// validate block align
	if (block_align != (bit_depth * channel_count) / 8)
	{
		set_error_message("Invalid block align.");
		return false;
	}
	// validate byte rate
	if (byte_rate_u32 / sample_rate_u32 != static_cast<unsigned int>(block_align))
	{
		set_error_message("Invalid byte rate.");
		return false;
	}
	channel_count_ = channel_count;
	src_byte_depth_ = bit_depth / 8;
	src_sample_rate_ = static_cast<int>(sample_rate_u32);
	return true;
}

void WavAudioDecoder::convert_u8(const unsigned char* bytes, int sample_count)
{
	for (int i = 0; i < sample_count; ++i)
	{
		const int s8 = bytes[i] - 128;
		cache_[i] = static_cast<float>(s8) / 128.0F;
	}
}

void WavAudioDecoder::convert_s16(const unsigned char* bytes, int sample_count)
{
	constexpr int sample_size = 2;
	for (int i = 0; i < sample_count; ++i)
	{
		const int s16 = endian::read_s16_le(&bytes[i * sample_size]);
		cache_[i] = static_cast<float>(s16) / 32768.0F;
	}
}

void WavAudioDecoder::convert_s24(const unsigned char* bytes, int sample_count)
{
	constexpr int sample_size = 3;
	for (int i = 0; i < sample_count; ++i)
	{
		const int s24 =
			(                         bytes[i * sample_size + 0]       ) |
			(                         bytes[i * sample_size + 1]  <<  8) |
			(static_cast<signed char>(bytes[i * sample_size + 2]) << 16);
		cache_[i] = static_cast<float>(s24) / 8388608.0F;
	}
}

void WavAudioDecoder::convert_s32(const unsigned char* bytes, int sample_count)
{
	constexpr int sample_size = 4;
	for (int i = 0; i < sample_count; ++i)
	{
		const int s32 = endian::read_s32_le(&bytes[i * sample_size]);
		cache_[i] = static_cast<float>(s32) / 2147483648.0F;
	}
}

void WavAudioDecoder::convert_f32_iec559(const unsigned char* bytes, int sample_count)
{
	constexpr int sample_size = 4;
	for (int i = 0; i < sample_count; ++i)
	{
		const float f32 = endian::read_f32_le(&bytes[i * sample_size]);
		cache_[i] = f32;
	}
}

int WavAudioDecoder::decode_frames_as_is(float* samples, int max_frames)
{
	int decoded_frame_count = 0;
	while (decoded_frame_count < max_frames)
	{
		if (!update_cache())
			break;
		const int frame_count = std::min(max_frames - decoded_frame_count, cache_frame_count_ - cache_frame_offset_);
		std::copy_n(
			&cache_[cache_frame_offset_ * channel_count_],
			frame_count * channel_count_,
			&samples[decoded_frame_count * channel_count_]);
		cache_frame_offset_ += frame_count;
		decoded_frame_count += frame_count;
	}
	return decoded_frame_count;
}

bool WavAudioDecoder::update_cache()
{
	if (cache_frame_offset_ < cache_frame_count_)
		return true;
	if (wav_offset_ >= wav_size_)
		return false;
	unsigned char buffer[byte_cache_capacity];
	const int byte_size = std::min(cache_capacity * channel_count_ * src_byte_depth_, wav_size_ - wav_offset_);
	BSTONE_ASSERT(byte_size <= byte_cache_capacity);
	if (!stream_->read_exactly(buffer, byte_size))
		return false;
	wav_offset_ += byte_size;
	const int sample_count = byte_size / src_byte_depth_;
	(this->*convert_samples_)(buffer, sample_count);
	cache_frame_offset_ = 0;
	cache_frame_count_ = sample_count / channel_count_;
	return true;
}

int WavAudioDecoder::decode_frames_with_resample(float* samples, int max_frames)
{
	int written_frame_count = 0;
	while (written_frame_count < max_frames)
	{
		if (!update_cache())
			break;
		std::copy_n(
			&cache_[cache_frame_offset_ * channel_count_],
			channel_count_,
			&samples[written_frame_count * channel_count_]);
		++written_frame_count;
		cache_frame_offset_counter_ += src_sample_rate_;
		while (cache_frame_offset_counter_ >= dst_sample_rate_)
		{
			cache_frame_offset_counter_ -= dst_sample_rate_;
			++cache_frame_offset_;
			if (!update_cache())
				break;
		}
	}
	return written_frame_count;
}

} // namespace

// =====================================

AudioDecoderUPtr make_wav_audio_decoder()
{
	return std::make_unique<WavAudioDecoder>();
}

} // namespace bstone
