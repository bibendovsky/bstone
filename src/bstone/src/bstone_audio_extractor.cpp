/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/


#include <algorithm>

#include "audio.h"
#include "id_ca.h"
#include "id_sd.h"

#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_extractor.h"
#include "bstone_binary_writer.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_sha1.h"
#include "bstone_string_helper.h"


namespace bstone
{

AudioExtractor::AudioExtractor() noexcept = default;

AudioExtractor::~AudioExtractor() = default;

// ==========================================================================

class AudioExtractorImpl final : public AudioExtractor
{
public:
	AudioExtractorImpl(AudioContentMgr& audio_content_mgr);
	~AudioExtractorImpl() override;

	void extract_music(const std::string& dst_dir) override;
	void extract_sfx(const std::string& dst_dir) override;

private:
	static constexpr auto pc_speaker_rate = 48'000;
	static constexpr auto wav_prefix_size = 44;

	using AudioChunkFilter = bool (*)(const AudioChunk& audio_chunk);

	using Sample = std::int16_t;
	using MusicNumbers = std::vector<int>;
	using DecodeBuffer = std::vector<Sample>;

	enum class ExtensionType
	{
		data,
		wav,
	}; // ExtensionType

	AudioContentMgr& audio_content_mgr_;
	DecodeBuffer decode_buffer_{};

	bool write_wav_header(int data_size, int bit_depth, int sample_rate, bstone::Stream& stream);
	void write_non_digitized_audio_chunk(const AudioChunk& sfx_info, bstone::Stream& stream);
	void write_digitized_audio_chunk(const AudioChunk& sfx_info, bstone::Stream& stream);

	static const char* make_file_name_prefix(AudioChunkType audio_chunk_type);
	static const char* make_file_extension(ExtensionType extension_type);
	static std::string make_number_string(int number);
	static std::string make_file_name(const AudioChunk& audio_chunk, ExtensionType extension_type);

	void extract_raw_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk);
	void extract_decoded_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk);
	void extract_audio_chunks(const std::string& dst_dir, const AudioChunkFilter audio_chunk_filter);
}; // AudioExtractorImpl

// --------------------------------------------------------------------------

decltype(AudioExtractorImpl::pc_speaker_rate) constexpr AudioExtractorImpl::pc_speaker_rate;

// --------------------------------------------------------------------------

AudioExtractorImpl::AudioExtractorImpl(
	AudioContentMgr& audio_content_mgr)
	:
	audio_content_mgr_{audio_content_mgr}
{
	decode_buffer_.resize(std::max(bstone::opl3_fixed_frequency, pc_speaker_rate));
}

AudioExtractorImpl::~AudioExtractorImpl() = default;

void AudioExtractorImpl::extract_music(const std::string& dst_dir)
{
	const auto audio_chunk_filter = [](const AudioChunk& audio_chunk) noexcept
	{
		return audio_chunk.type == AudioChunkType::adlib_music && audio_chunk.data;
	};

	extract_audio_chunks(dst_dir, audio_chunk_filter);
}

void AudioExtractorImpl::extract_sfx(const std::string& dst_dir)
{
	const auto audio_chunk_filter = [](const AudioChunk& audio_chunk) noexcept
	{
		return audio_chunk.type != AudioChunkType::adlib_music && audio_chunk.data;
	};

	extract_audio_chunks(dst_dir, audio_chunk_filter);
}

bool AudioExtractorImpl::write_wav_header(int data_size, int bit_depth, int sample_rate, bstone::Stream& stream)
{
	const auto aligned_data_size = ((data_size + 1) / 2) * 2;
	const auto wav_size = aligned_data_size + wav_prefix_size;
	const auto audio_format = 1; // PCM
	const auto channel_count = 1;
	const auto byte_depth = bit_depth / 8;
	const auto byte_rate = sample_rate * channel_count * byte_depth;
	const auto block_align = channel_count * byte_depth;
	auto writer = bstone::BinaryWriter{stream};
	auto result = true;
	writer.write_u32(bstone::endian::to_big(0x52494646)); // "RIFF"
	// riff_chunk_size = = "file size" - "chunk id" + "chunk size".
	const auto riff_chunk_size = static_cast<std::uint32_t>(wav_size - 4 - 4);
	writer.write_u32(bstone::endian::to_little(riff_chunk_size)); // Chunk size.
	writer.write_u32(bstone::endian::to_big(0x57415645)); // "WAVE"
	writer.write_u32(bstone::endian::to_big(0x666D7420)); // "fmt "
	writer.write_u32(bstone::endian::to_little(16)); // Format size.
	writer.write_u16(bstone::endian::to_little(static_cast<std::uint16_t>(audio_format))); // Audio format.
	writer.write_u16(bstone::endian::to_little(static_cast<std::uint16_t>(channel_count))); // Channel count.
	writer.write_u32(bstone::endian::to_little(sample_rate)); // Sample rate.
	writer.write_u32(bstone::endian::to_little(static_cast<std::uint32_t>(byte_rate))); // Byte rate.
	writer.write_u16(bstone::endian::to_little(static_cast<std::uint16_t>(block_align))); // Block align.
	writer.write_u16(bstone::endian::to_little(static_cast<std::uint16_t>(bit_depth))); // Bits per sample.
	writer.write_u32(bstone::endian::to_big(0x64617461)); // "data"
	writer.write_u32(bstone::endian::to_little(static_cast<std::uint32_t>(data_size))); // Data size.
	return result;
}

void AudioExtractorImpl::write_non_digitized_audio_chunk(const AudioChunk& audio_chunk, bstone::Stream& stream)
{
	auto audio_decoder_type = AudioDecoderType{};
	auto dst_rate = 0;

	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_music:
			audio_decoder_type = AudioDecoderType::adlib_music;
			dst_rate = bstone::opl3_fixed_frequency;
			break;

		case AudioChunkType::adlib_sfx:
			audio_decoder_type = AudioDecoderType::adlib_sfx;
			dst_rate = bstone::opl3_fixed_frequency;
			break;

		case AudioChunkType::pc_speaker:
			audio_decoder_type = AudioDecoderType::pc_speaker;
			dst_rate = pc_speaker_rate;
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported audio chunk type.");
	}

	auto audio_decoder = bstone::make_audio_decoder(audio_decoder_type, Opl3Type::dbopl);

	if (!audio_decoder)
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to create decoder.");
	}

	auto param = bstone::AudioDecoderInitParam{};
	param.src_raw_data_ = audio_chunk.data;
	param.src_raw_size_ = audio_chunk.data_size;
	param.dst_rate_ = dst_rate;

	if (!audio_decoder->initialize(param))
	{
		BSTONE_THROW_STATIC_SOURCE("Failed to initialize decoder.");
	}

	stream.set_position(wav_prefix_size);

	constexpr auto sample_size = static_cast<int>(sizeof(Sample));
	constexpr auto bit_depth = sample_size * 8;

	auto data_size = 0;
	auto sample_count = 0;
	auto abs_max_sample = 0;

	while (true)
	{
		const auto decoded_count = audio_decoder->decode(dst_rate, decode_buffer_.data());

		if (decoded_count == 0)
		{
			break;
		}

		for (auto i = 0; i < decoded_count; ++i)
		{
			const auto sample = static_cast<int>(decode_buffer_[i]);
			abs_max_sample = std::max(std::abs(sample), abs_max_sample);
		}

		const auto decoded_size = decoded_count * sample_size;

		stream.write_exactly(decode_buffer_.data(), decoded_size);

		data_size += decoded_size;
		sample_count += decoded_count;
	}

	stream.set_position(0);

	if (!write_wav_header(data_size, bit_depth, bstone::opl3_fixed_frequency, stream))
	{
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	}

	const auto volume_factor = 32'767.0 / abs_max_sample;

	bstone::globals::logger->log_information(("\tSample rate: " + std::to_string(dst_rate)).c_str());
	bstone::globals::logger->log_information(("\tSample count: " + std::to_string(sample_count)).c_str());
	bstone::globals::logger->log_information(("\tVolume factor: " + std::to_string(volume_factor)).c_str());
}

void AudioExtractorImpl::write_digitized_audio_chunk(const AudioChunk& audio_chunk, bstone::Stream& stream)
{
	constexpr auto sample_size = 1;
	constexpr auto bit_depth = sample_size * 8;
	const auto data_size = audio_chunk.data_size;

	if (!write_wav_header(data_size, bit_depth, bstone::audio_decoder_w3d_pcm_frequency, stream))
	{
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	}

	stream.write_exactly(audio_chunk.data, data_size);

	if ((data_size % 2) != 0)
	{
		const auto zero_octet = std::uint8_t{};
		stream.write_exactly(&zero_octet, 1);
	}

	auto abs_max_sample = 0;

	if (data_size > 0)
	{
		const auto pcm_u8_data = audio_chunk.data;

		for (auto i = 0; i < data_size; ++i)
		{
			abs_max_sample = std::max(std::abs(pcm_u8_data[i] - 128), abs_max_sample);
		}
	}

	const auto volume_factor = 127.0 / abs_max_sample;

	bstone::globals::logger->log_information(
		("\tSample rate: " + std::to_string(bstone::audio_decoder_w3d_pcm_frequency)).c_str());
	bstone::globals::logger->log_information(("\tSample count: " + std::to_string(data_size)).c_str());
	bstone::globals::logger->log_information(("\tVolume factor: " + std::to_string(volume_factor)).c_str());
}

const char* AudioExtractorImpl::make_file_name_prefix(AudioChunkType audio_chunk_type)
{
	switch (audio_chunk_type)
	{
		case AudioChunkType::adlib_music: return "music_adlib";
		case AudioChunkType::adlib_sfx: return "sfx_adlib";
		case AudioChunkType::pc_speaker: return "sfx_pc_speaker";
		case AudioChunkType::digitized: return "sfx_digitized";
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported audio chunk type.");
	}
}

const char* AudioExtractorImpl::make_file_extension(ExtensionType extension_type)
{
	switch (extension_type)
	{
		case ExtensionType::data: return ".data";
		case ExtensionType::wav: return ".wav";
		default: BSTONE_THROW_STATIC_SOURCE("Unsupported extension type.");
	}
}

std::string AudioExtractorImpl::make_number_string(int number)
{
	BSTONE_ASSERT(number >= 0);
	return StringHelper::make_left_padded_with_zero(number, 8);
}

std::string AudioExtractorImpl::make_file_name(const AudioChunk& audio_chunk, ExtensionType extension_type)
{
	const auto file_name_prefix = make_file_name_prefix(audio_chunk.type);
	const auto number_string = make_number_string(audio_chunk.audio_index);
	const auto file_extension = make_file_extension(extension_type);
	const auto file_name = std::string{} + file_name_prefix + '_' + number_string + file_extension;
	return file_name;
}

void AudioExtractorImpl::extract_raw_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk)
{
	const auto file_name = make_file_name(audio_chunk, ExtensionType::data);
	globals::logger->log_information(file_name.c_str());
	const auto dst_file_name = fs_utils::append_path(dst_dir, file_name);

	auto file_stream = FileStream{
		dst_file_name.c_str(),
		FileOpenFlags::create | FileOpenFlags::truncate | FileOpenFlags::write,
		FileShareMode::exclusive};

	const auto written_size = file_stream.write(audio_chunk.data, audio_chunk.data_size);

	if (written_size != audio_chunk.data_size)
	{
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	}

	auto sha1 = Sha1{};
	sha1.process(audio_chunk.data, audio_chunk.data_size);
	sha1.finish();
	const auto sha1_string = array_to_hex_string(sha1.get_digest());

	globals::logger->log_information(("\tSHA1: " + sha1_string).c_str());
}

void AudioExtractorImpl::extract_decoded_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk)
{
	const auto file_name = make_file_name(audio_chunk, ExtensionType::wav);
	globals::logger->log_information(file_name.c_str());
	const auto dst_file_name = fs_utils::append_path(dst_dir, file_name);

	auto file_stream = FileStream{
		dst_file_name.c_str(),
		FileOpenFlags::create | FileOpenFlags::truncate | FileOpenFlags::write,
		FileShareMode::exclusive};

	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_music:
		case AudioChunkType::adlib_sfx:
		case AudioChunkType::pc_speaker:
			write_non_digitized_audio_chunk(audio_chunk, file_stream);
			break;

		case AudioChunkType::digitized:
			write_digitized_audio_chunk(audio_chunk, file_stream);
			break;

		default:
			BSTONE_THROW_STATIC_SOURCE("Unsupported audio chunk type.");
	}
}

void AudioExtractorImpl::extract_audio_chunks(const std::string& dst_dir, const AudioChunkFilter audio_chunk_filter)
{
	const auto audio_chunk_count = audio_content_mgr_.get_chunk_count();

	for (auto i = 0; i < audio_chunk_count; ++i)
	{
		const auto& audio_chunk = audio_content_mgr_.get_chunk(i);

		if (audio_chunk_filter(audio_chunk))
		{
			extract_raw_audio_chunk(dst_dir, audio_chunk);
			extract_decoded_audio_chunk(dst_dir, audio_chunk);
		}
	}
}

// ==========================================================================

AudioExtractorUPtr make_audio_extractor(AudioContentMgr& audio_content_mgr)
{
	return std::make_unique<AudioExtractorImpl>(audio_content_mgr);
}

} // bstone
