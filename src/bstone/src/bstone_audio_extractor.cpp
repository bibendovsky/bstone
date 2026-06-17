/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "audio.h"
#include "id_ca.h"
#include "id_sd.h"
#include "bstone_assert.h"
#include "bstone_audio_decoder.h"
#include "bstone_audio_extractor.h"
#include "bstone_audio_mixer_utils.h"
#include "bstone_audio_sample_converter.h"
#include "bstone_endian.h"
#include "bstone_exception.h"
#include "bstone_format.h"
#include "bstone_fs_utils.h"
#include "bstone_globals.h"
#include "bstone_logger.h"
#include "bstone_memory_binary_writer.h"
#include "bstone_sha1.h"
#include "bstone_string_helper.h"
#include "bstone_sys_fs.h"
#include <algorithm>
#include <vector>
#include <format>

namespace bstone {

class AudioExtractorImpl final : public AudioExtractor
{
public:
	AudioExtractorImpl(AudioContentMgr& audio_content_mgr);
	~AudioExtractorImpl() override;

	void extract_music(const std::string& dst_dir) override;
	void extract_sfx(const std::string& dst_dir) override;

private:
	inline static constexpr int pc_speaker_rate = 48'000;
	inline static constexpr int wav_prefix_size = 44;

	using AudioChunkFilter = bool (*)(const AudioChunk& audio_chunk);

	using MusicNumbers = std::vector<int>;
	using SamplesS16 = std::vector<short>;
	using SamplesF32 = std::vector<float>;

	enum class ExtensionType
	{
		data,
		wav,
	};

	AudioContentMgr& audio_content_mgr_;
	SamplesS16 samples_s16_{};
	SamplesF32 samples_f32_{};

	bool write_wav_header(int data_size, int bit_depth, int sample_rate, bstone::Stream& stream);
	void write_non_digitized_audio_chunk(const AudioChunk& sfx_info, bstone::Stream& stream, Opl3Type opl3_type);
	void write_digitized_audio_chunk(const AudioChunk& sfx_info, bstone::Stream& stream);
	static std::string make_file_name(const AudioChunk& audio_chunk, ExtensionType extension_type, Opl3Type opl3_type);
	void extract_raw_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk);
	void extract_decoded_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk);
	void extract_audio_chunks(const std::string& dst_dir, const AudioChunkFilter audio_chunk_filter);
};

// -------------------------------------

AudioExtractorImpl::AudioExtractorImpl(AudioContentMgr& audio_content_mgr)
	:
	audio_content_mgr_{audio_content_mgr}
{
	const int capacity = std::max(bstone::opl3_fixed_frequency, pc_speaker_rate);
	samples_s16_.resize(capacity);
	samples_f32_.resize(capacity);
}

AudioExtractorImpl::~AudioExtractorImpl() = default;

void AudioExtractorImpl::extract_music(const std::string& dst_dir)
{
	const auto audio_chunk_filter = [](const AudioChunk& audio_chunk)
	{
		return audio_chunk.type == AudioChunkType::adlib_music && audio_chunk.data;
	};
	extract_audio_chunks(dst_dir, audio_chunk_filter);
}

void AudioExtractorImpl::extract_sfx(const std::string& dst_dir)
{
	const auto audio_chunk_filter = [](const AudioChunk& audio_chunk)
	{
		return audio_chunk.type != AudioChunkType::adlib_music && audio_chunk.data;
	};
	extract_audio_chunks(dst_dir, audio_chunk_filter);
}

bool AudioExtractorImpl::write_wav_header(int data_size, int bit_depth, int sample_rate, bstone::Stream& stream)
{
	const int aligned_data_size = ((data_size + 1) / 2) * 2;
	const int wav_size = aligned_data_size + wav_prefix_size;
	const int riff_chunk_size = wav_size - (4 + 4); // file_size - chunk_header_size
	const int audio_format = 1; // PCM
	const int channel_count = 1;
	const int byte_depth = bit_depth / 8;
	const int byte_rate = sample_rate * channel_count * byte_depth;
	const int block_align = channel_count * byte_depth;
	constexpr const char* const riff_fourcc = "RIFF";
	constexpr const char* const wave_fourcc = "WAVE";
	constexpr const char* const fmt0x20_fourcc = "fmt ";
	constexpr const char* const data_fourcc = "data";
	unsigned char wav_prefix[wav_prefix_size];
	MemoryBinaryWriter writer{wav_prefix, wav_prefix_size};
	writer.write(riff_fourcc, 4); // "RIFF"
	writer.write_u32_le(static_cast<unsigned int>(riff_chunk_size)); // Chunk size.
	writer.write(wave_fourcc, 4); // "WAVE"
	writer.write(fmt0x20_fourcc, 4); // "fmt "
	writer.write_u32_le(16); // Format size.
	writer.write_u16_le(static_cast<unsigned short>(audio_format)); // Audio format.
	writer.write_u16_le(static_cast<unsigned short>(channel_count)); // Channel count.
	writer.write_u32_le(static_cast<unsigned int>(sample_rate)); // Sample rate.
	writer.write_u32_le(static_cast<unsigned int>(byte_rate)); // Byte rate.
	writer.write_u16_le(static_cast<unsigned short>(block_align)); // Block align.
	writer.write_u16_le(static_cast<unsigned short>(bit_depth)); // Bits per sample.
	writer.write(data_fourcc, 4); // "data"
	writer.write_u32_le(static_cast<unsigned int>(data_size)); // Data size.
	return stream.write(wav_prefix, wav_prefix_size) == wav_prefix_size;
}

void AudioExtractorImpl::write_non_digitized_audio_chunk(const AudioChunk& audio_chunk, bstone::Stream& stream, Opl3Type opl3_type)
{
	AudioDecoderType audio_decoder_type{};
	int dst_rate = 0;
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
	AudioDecoderUPtr audio_decoder = bstone::make_audio_decoder(audio_decoder_type, opl3_type);
	if (audio_decoder == nullptr)
		BSTONE_THROW_STATIC_SOURCE("Failed to create decoder.");
	bstone::AudioDecoderInitParam param{
		.src_raw_data = audio_chunk.data,
		.src_raw_size = audio_chunk.data_size,
		.dst_rate = dst_rate};
	if (!audio_decoder->initialize(param))
		BSTONE_THROW_STATIC_SOURCE("Failed to initialize decoder.");
	stream.set_position(wav_prefix_size);
	constexpr int sample_size = 2;
	constexpr int bit_depth = sample_size * 8;
	int data_size = 0;
	int sample_count = 0;
	float abs_max_sample = 0.0F;
	for (;;)
	{
		const int decoded_count = audio_decoder->decode(dst_rate, samples_f32_.data());
		if (decoded_count == 0)
			break;
		for (int i = 0; i < decoded_count; ++i)
		{
			const float sample_f32 = samples_f32_[i];
			abs_max_sample = std::max(std::abs(sample_f32), abs_max_sample);
			const std::int16_t sample_s16 = AudioSampleConverter::f32_to_s16(sample_f32);
			endian::write_s16_le(sample_s16, samples_s16_.data() + i);
		}
		const int decoded_size = decoded_count * sample_size;
		stream.write_exactly(samples_s16_.data(), decoded_size);
		data_size += decoded_size;
		sample_count += decoded_count;
	}
	stream.set_position(0);
	if (!write_wav_header(data_size, bit_depth, bstone::opl3_fixed_frequency, stream))
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	const double volume_factor = 32'767.0 / abs_max_sample;
	bstone::globals::logger->log_information("\tSample rate: {}", dst_rate);
	bstone::globals::logger->log_information("\tSample count: {}", sample_count);
	bstone::globals::logger->log_information("\tVolume factor: {}", volume_factor);
}

void AudioExtractorImpl::write_digitized_audio_chunk(const AudioChunk& audio_chunk, bstone::Stream& stream)
{
	constexpr int sample_size = 1;
	constexpr int bit_depth = sample_size * 8;
	const int data_size = audio_chunk.data_size;
	if (!write_wav_header(data_size, bit_depth, bstone::audio_decoder_w3d_pcm_frequency, stream))
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	stream.write_exactly(audio_chunk.data, data_size);
	if ((data_size % 2) != 0)
	{
		const auto zero_octet = std::uint8_t{};
		stream.write_exactly(&zero_octet, 1);
	}
	int abs_max_sample = 0;
	if (data_size > 0)
	{
		const unsigned char* const pcm_u8_data = audio_chunk.data;
		for (int i = 0; i < data_size; ++i)
			abs_max_sample = std::max(std::abs(pcm_u8_data[i] - 128), abs_max_sample);
	}
	const double volume_factor = 127.0 / abs_max_sample;
	bstone::globals::logger->log_information("\tSample rate: {}", bstone::audio_decoder_w3d_pcm_frequency);
	bstone::globals::logger->log_information("\tSample count: {}", data_size);
	bstone::globals::logger->log_information("\tVolume factor: {}", volume_factor);
}

std::string AudioExtractorImpl::make_file_name(const AudioChunk& audio_chunk, ExtensionType extension_type, Opl3Type opl3_type)
{
	std::string filename{};
	filename.reserve(256);
	const AssetsInfo& assets_info = get_assets_info();
	if (audio_chunk.type == AudioChunkType::adlib_music)
		AudioMixerUtils::append_music_chunk_dirname(assets_info, filename);
	else
		AudioMixerUtils::append_sfx_chunk_dirname(assets_info, filename);
	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_music:
		case AudioChunkType::adlib_sfx:
			fs_utils::append_path_inplace(filename, "adlib");
			break;
		case AudioChunkType::pc_speaker:
			fs_utils::append_path_inplace(filename, "pc_speaker");
			break;
		case AudioChunkType::digitized:
			fs_utils::append_path_inplace(filename, "digitized");
			break;
		default:
			BSTONE_ASSERT(false && "Unsupported audio chunk type.");
			fs_utils::append_path_inplace(filename, "?");
			break;
	}
	if (extension_type == ExtensionType::data)
		fs_utils::append_path_inplace(filename, "raw");
	switch (opl3_type)
	{
		case Opl3Type::none:
			break;
		case Opl3Type::dbopl:
			fs_utils::append_path_inplace(filename, "dosbox");
			break;
		case Opl3Type::nuked:
			fs_utils::append_path_inplace(filename, "nuked");
			break;
		default:
			BSTONE_ASSERT(false && "Unknown OPL3 type.");
			fs_utils::append_path_inplace(filename, "?");
			break;
	}
	std::string_view chunk_name{};
	if (audio_chunk.type == AudioChunkType::adlib_music)
		chunk_name = AudioMixerUtils::get_music_chunk_name(audio_chunk.audio_index, assets_info);
	else
		chunk_name = AudioMixerUtils::get_sfx_chunk_name(audio_chunk.audio_index, assets_info);
	fs_utils::append_path_inplace(filename, chunk_name);
	switch (extension_type)
	{
		case ExtensionType::data:
			filename += ".data";
			break;
		case ExtensionType::wav:
			filename += ".wav";
			break;
		default:
			BSTONE_ASSERT(false && "Unsupported extension type.");
			filename += ".?";
			break;
	}
	return filename;
}

void AudioExtractorImpl::extract_raw_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk)
{
	const std::string file_name = make_file_name(audio_chunk, ExtensionType::data, Opl3Type::none);
	globals::logger->log_information(file_name.c_str());
	const std::string dst_file_name = fs_utils::append_path(dst_dir, file_name);
	const std::string dirname = fs_utils::get_dirname(dst_file_name);
	sys::create_directories(dirname.c_str());
	FileStream file_stream{dst_file_name.c_str(), sys::FileMode::create};
	if (!file_stream.is_open())
	{
		std::string error_message{};
		error_message.reserve(1024);
		error_message += "Failed to open a file \"";
		error_message += dst_file_name;
		error_message += "\".";
		BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
	}
	const std::intptr_t written_size = file_stream.write(audio_chunk.data, audio_chunk.data_size);
	if (written_size != audio_chunk.data_size)
		BSTONE_THROW_STATIC_SOURCE("Write error.");
	Sha1 sha1{};
	sha1.process(audio_chunk.data, audio_chunk.data_size);
	sha1.finish();
	const Sha1Digest& sha1_digest = sha1.get_digest();
	const std::string sha1_string = StringHelper::bytes_to_hex_string(sha1_digest.get_data(), sha1_digest.get_size());
	globals::logger->log_information("\tSHA1: {}", sha1_string);
}

void AudioExtractorImpl::extract_decoded_audio_chunk(const std::string& dst_dir, const AudioChunk& audio_chunk)
{
	using Opl3Types = std::vector<Opl3Type>;
	Opl3Types opl3_types{};
	opl3_types.reserve(2);
	switch (audio_chunk.type)
	{
		case AudioChunkType::adlib_music:
		case AudioChunkType::adlib_sfx:
			opl3_types.emplace_back(Opl3Type::dbopl);
			opl3_types.emplace_back(Opl3Type::nuked);
			break;
		default:
			opl3_types.emplace_back(Opl3Type::none);
			break;
	}
	for (Opl3Type opl3_type : opl3_types)
	{
		const std::string file_name = make_file_name(audio_chunk, ExtensionType::wav, opl3_type);
		globals::logger->log_information(file_name.c_str());
		const std::string dst_file_name = fs_utils::append_path(dst_dir, file_name);
		const std::string dirname = fs_utils::get_dirname(dst_file_name);
		sys::create_directories(dirname.c_str());
		FileStream file_stream{dst_file_name.c_str(), sys::FileMode::create};
		if (!file_stream.is_open())
		{
			std::string error_message{};
			error_message.reserve(1024);
			error_message += "Failed to open a file \"";
			error_message += dst_file_name;
			error_message += "\".";
			BSTONE_THROW_DYNAMIC_SOURCE(error_message.c_str());
		}
		switch (audio_chunk.type)
		{
			case AudioChunkType::adlib_music:
			case AudioChunkType::adlib_sfx:
			case AudioChunkType::pc_speaker:
				write_non_digitized_audio_chunk(audio_chunk, file_stream, opl3_type);
				break;
			case AudioChunkType::digitized:
				write_digitized_audio_chunk(audio_chunk, file_stream);
				break;
			default:
				BSTONE_THROW_STATIC_SOURCE("Unsupported audio chunk type.");
		}
	}
}

void AudioExtractorImpl::extract_audio_chunks(const std::string& dst_dir, AudioChunkFilter audio_chunk_filter)
{
	const int audio_chunk_count = audio_content_mgr_.get_chunk_count();
	for (int i = 0; i < audio_chunk_count; ++i)
	{
		const AudioChunk& audio_chunk = audio_content_mgr_.get_chunk(i);
		if (audio_chunk_filter(audio_chunk))
		{
			extract_raw_audio_chunk(dst_dir, audio_chunk);
			extract_decoded_audio_chunk(dst_dir, audio_chunk);
		}
	}
}

// =====================================

AudioExtractorUPtr make_audio_extractor(AudioContentMgr& audio_content_mgr)
{
	return std::make_unique<AudioExtractorImpl>(audio_content_mgr);
}

} // namespace bstone
