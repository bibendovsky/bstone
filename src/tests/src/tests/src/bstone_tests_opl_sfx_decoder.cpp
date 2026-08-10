#include <cstdint>
#include <cstring>

#include <limits>
#include <vector>

#include "bstone_endian.h"
#include "bstone_opl_sfx_decoder.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// The decoder requires a chunk to hold the 23-octet AdLibSound header, the
// commands, and the leading length field counted a second time.
constexpr int min_chunk_size = 27;

constexpr auto test_emulator_type = bstone::OplEmulatorType::dbopl;

using Chunk = std::vector<std::uint8_t>;

// Makes an OPL SFX chunk with a command count of `sfx_length` and room for
// `command_count` commands.
Chunk make_chunk(std::uint32_t sfx_length, int command_count)
{
	auto chunk = Chunk(static_cast<std::size_t>(min_chunk_size + command_count));
	bstone::endian::write_u32_le(sfx_length, chunk.data());
	chunk[12] = 0x0F; // Modulator sustain.
	chunk[13] = 0x0F; // Carrier sustain.
	chunk[22] = 0x02; // Block.

	for (int i = 0; i < command_count; ++i)
	{
		chunk[static_cast<std::size_t>(23 + i)] = 0x40;
	}

	return chunk;
}

// ==========================================================================

// bool initialize(const AudioDecoderInitParam&)
void test_iio7snjvd13n45mq()
{
	const auto chunk = make_chunk(2, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(is_initialized && decoder->get_channel_count() > 0);
}

// bool initialize(const AudioDecoderInitParam&)
// A command count that overruns the chunk is rejected.
void test_1qv6tag86x6csx62()
{
	const auto chunk = make_chunk(3, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(!is_initialized && !decoder->is_initialized());
}

// bool initialize(const AudioDecoderInitParam&)
// A command count big enough to overflow the bounds check is rejected.
void test_7zmclziabp1uoi0t()
{
	const auto chunk = make_chunk(
		static_cast<std::uint32_t>(std::numeric_limits<int>::max()), 2);

	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	const auto is_valid =
		!is_initialized &&
		std::strcmp(decoder->get_error_message(), "Command count mismatch.") == 0;

	tester.check(is_valid);
}

// bool initialize(const AudioDecoderInitParam&)
// A command count with the sign bit set is rejected.
void test_vbe1vlwzo68ez2we()
{
	const auto chunk = make_chunk(0x80000000U, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(!is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
// An empty command stream is rejected.
void test_yl2h5m7aaycrxla7()
{
	const auto chunk = make_chunk(0, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(!is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
// A chunk too small to hold the length field is rejected.
void test_d1ra96j5lh0dpym3()
{
	const auto chunk = make_chunk(2, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = 3,
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(!is_initialized);
}

// bool initialize(const AudioDecoderInitParam&)
// An instrument with no sustain at all is rejected.
void test_rj4j6846djiip7kv()
{
	auto chunk = make_chunk(2, 2);
	chunk[12] = 0;
	chunk[13] = 0;
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	tester.check(!is_initialized);
}

// ==========================================================================

// int decode_frames(float*, int)
void test_xn3uft1b86gvchbn()
{
	constexpr int max_frames = 512;
	const auto chunk = make_chunk(2, 2);
	auto decoder = bstone::make_opl_sfx_audio_decoder(test_emulator_type);

	const auto is_initialized = decoder->initialize(
		bstone::AudioDecoderInitParam{
			.vfs_stream = bstone::VfsInputStreamUPtr{},
			.src_raw_data = chunk.data(),
			.src_raw_size = static_cast<int>(chunk.size()),
			.dst_rate = bstone::OplEmulator::fixed_sample_rate});

	if (!is_initialized)
	{
		tester.check(false);
		return;
	}

	auto samples = std::vector<float>(
		static_cast<std::size_t>(max_frames * decoder->get_channel_count()));

	const auto frame_count = decoder->decode_frames(samples.data(), max_frames);
	tester.check(frame_count == max_frames);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_initialize();
		register_decode_frames();
	}

private:
	void register_initialize()
	{
		tester.register_test("OplSfxDecoder#iio7snjvd13n45mq", test_iio7snjvd13n45mq);
		tester.register_test("OplSfxDecoder#1qv6tag86x6csx62", test_1qv6tag86x6csx62);
		tester.register_test("OplSfxDecoder#7zmclziabp1uoi0t", test_7zmclziabp1uoi0t);
		tester.register_test("OplSfxDecoder#vbe1vlwzo68ez2we", test_vbe1vlwzo68ez2we);
		tester.register_test("OplSfxDecoder#yl2h5m7aaycrxla7", test_yl2h5m7aaycrxla7);
		tester.register_test("OplSfxDecoder#d1ra96j5lh0dpym3", test_d1ra96j5lh0dpym3);
		tester.register_test("OplSfxDecoder#rj4j6846djiip7kv", test_rj4j6846djiip7kv);
	}

	void register_decode_frames()
	{
		tester.register_test("OplSfxDecoder#xn3uft1b86gvchbn", test_xn3uft1b86gvchbn);
	}
};

auto registrator = Registrator{};

} // namespace
