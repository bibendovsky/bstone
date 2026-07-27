#include <cstdint>

#include <vector>

#include "bstone_pc_speaker_audio_decoder.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// A rate that is an exact multiple of the 140 Hz command rate, so that every
// command lasts a whole number of frames.
constexpr int test_dst_rate = 4200;
constexpr int test_frames_per_command = test_dst_rate / 140;

using Chunk = std::vector<std::uint8_t>;

// Makes a PC Speaker chunk: a 32-bit little-endian command count, two octets
// of padding and then one octet per command.
Chunk make_chunk(std::uint32_t command_count, int command_octet_count)
{
	auto chunk = Chunk(static_cast<std::size_t>(6 + command_octet_count));
	chunk[0] = static_cast<std::uint8_t>(command_count);
	chunk[1] = static_cast<std::uint8_t>(command_count >> 8);
	chunk[2] = static_cast<std::uint8_t>(command_count >> 16);
	chunk[3] = static_cast<std::uint8_t>(command_count >> 24);

	for (int i = 0; i < command_octet_count; ++i)
		chunk[static_cast<std::size_t>(6 + i)] = static_cast<std::uint8_t>(0x40 + i);

	return chunk;
}

// ==========================================================================

// bool initialize(const AudioDecoderInitParam&)
void test_80rrg3cr9xrcamri()
{
	const auto chunk = make_chunk(2, 2);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	tester.check(is_initialized && decoder->get_channel_count() == 1);
}

// bool initialize(const AudioDecoderInitParam&)
// A command count that overruns the chunk is rejected.
void test_ap8nzhx4xjbnj9gq()
{
	const auto chunk = make_chunk(3, 2);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	tester.check(!is_initialized && !decoder->is_initialized());
}

// bool initialize(const AudioDecoderInitParam&)
// A command count with the sign bit set is rejected.
void test_9v6lt36c9wejy81l()
{
	const auto chunk = make_chunk(0xFFFFFFFFU, 2);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	tester.check(!is_initialized && !decoder->is_initialized());
}

// bool initialize(const AudioDecoderInitParam&)
// A chunk with a trailing terminator octet, as the shipped ones have.
void test_o08xhaxbuvfilq7q()
{
	const auto chunk = make_chunk(2, 3);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	tester.check(is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
// A chunk too small to hold the header is rejected.
void test_6v5ultp3usjc29fj()
{
	const auto chunk = make_chunk(0, 0);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = 5,
			.dst_rate = test_dst_rate});

	tester.check(!is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
void test_d4xdxn8nf9gux1z8()
{
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = nullptr,
			.src_raw_size = 16,
			.dst_rate = test_dst_rate});

	tester.check(!is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
// A destination rate at or below the command rate is rejected.
void test_ywodxzuk80x287hf()
{
	const auto chunk = make_chunk(2, 2);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = 140});

	tester.check(!is_initialized);
}

// ==========================================================================

// int decode_frames(float*, int)
void test_4e4mervbv9g39vqa()
{
	constexpr int command_count = 2;
	const auto chunk = make_chunk(command_count, command_count);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	auto samples = std::vector<float>(test_dst_rate);
	const auto frame_count = decoder->decode_frames(samples.data(), test_dst_rate);
	const auto tail_frame_count = decoder->decode_frames(samples.data(), test_dst_rate);

	const auto is_valid =
		is_initialized &&
		frame_count == command_count * test_frames_per_command &&
		tail_frame_count == 0;

	tester.check(is_valid);
}

// ==========================================================================

// bool rewind()
// A rewound sound plays back exactly as it did the first time.
void test_wujzbduytphe9hvl()
{
	constexpr int command_count = 2;
	const auto chunk = make_chunk(command_count, command_count);
	auto decoder = bstone::make_pc_speaker_audio_decoder();

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = test_dst_rate});

	auto first_samples = std::vector<float>(test_dst_rate);
	const auto first_frame_count = decoder->decode_frames(first_samples.data(), test_dst_rate);
	const auto is_rewound = decoder->rewind();
	auto second_samples = std::vector<float>(test_dst_rate);
	const auto second_frame_count = decoder->decode_frames(second_samples.data(), test_dst_rate);

	const auto is_valid =
		is_initialized &&
		is_rewound &&
		first_frame_count == command_count * test_frames_per_command &&
		second_frame_count == first_frame_count &&
		first_samples == second_samples;

	tester.check(is_valid);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_initialize();
		register_decode_frames();
		register_rewind();
	}

private:
	void register_initialize()
	{
		tester.register_test("PcSpeakerAudioDecoder#80rrg3cr9xrcamri", test_80rrg3cr9xrcamri);
		tester.register_test("PcSpeakerAudioDecoder#ap8nzhx4xjbnj9gq", test_ap8nzhx4xjbnj9gq);
		tester.register_test("PcSpeakerAudioDecoder#9v6lt36c9wejy81l", test_9v6lt36c9wejy81l);
		tester.register_test("PcSpeakerAudioDecoder#o08xhaxbuvfilq7q", test_o08xhaxbuvfilq7q);
		tester.register_test("PcSpeakerAudioDecoder#6v5ultp3usjc29fj", test_6v5ultp3usjc29fj);
		tester.register_test("PcSpeakerAudioDecoder#d4xdxn8nf9gux1z8", test_d4xdxn8nf9gux1z8);
		tester.register_test("PcSpeakerAudioDecoder#ywodxzuk80x287hf", test_ywodxzuk80x287hf);
	}

	void register_decode_frames()
	{
		tester.register_test("PcSpeakerAudioDecoder#4e4mervbv9g39vqa", test_4e4mervbv9g39vqa);
	}

	void register_rewind()
	{
		tester.register_test("PcSpeakerAudioDecoder#wujzbduytphe9hvl", test_wujzbduytphe9hvl);
	}
};

auto registrator = Registrator{};

} // namespace
