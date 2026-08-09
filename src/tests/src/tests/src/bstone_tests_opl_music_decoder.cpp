#include <cstdint>

#include <initializer_list>
#include <vector>

#include "bstone_audio_decoder.h"
#include "bstone_endian.h"
#include "bstone_opl_music_decoder.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto dst_rate = 44100;

// An OPL music chunk is a little-endian u16 with the size of the command block
// in bytes, followed by that many bytes of four-byte commands: a register
// address, a register value and a little-endian u16 delay in ticks.
using Chunk = std::vector<std::uint8_t>;

Chunk make_chunk(int command_block_size, std::initializer_list<std::uint8_t> commands)
{
	auto chunk = Chunk(2);
	bstone::endian::write_u16_le(static_cast<std::uint16_t>(command_block_size), chunk.data());
	chunk.insert(chunk.end(), commands);
	return chunk;
}

bstone::AudioDecoderUPtr make_decoder(const Chunk& chunk)
{
	auto decoder = bstone::make_opl_music_audio_decoder(bstone::OplEmulatorType::dbopl);

	const auto param = bstone::AudioDecoderInitParam
	{
		.vfs_stream = bstone::VfsInputStreamUPtr{},
		.src_raw_data = chunk.data(),
		.src_raw_size = static_cast<int>(chunk.size()),
		.dst_rate = dst_rate};

	if (!decoder->initialize(param))
	{
		return nullptr;
	}

	return decoder;
}

// ==========================================================================

// bool initialize(const AudioDecoderInitParam&)
// Too small for the command block size.
void test_q7m2vh4nx8ktz3pd()
{
	const auto chunk = Chunk{0x00};
	const auto decoder = make_decoder(chunk);
	tester.check(decoder == nullptr);
}

// bool initialize(const AudioDecoderInitParam&)
// The command block size is not a multiple of the command size.
void test_w9c5rj1bstn6yl0a()
{
	const auto chunk = make_chunk(3, {0x01, 0x20, 0x00});
	const auto decoder = make_decoder(chunk);
	tester.check(decoder == nullptr);
}

// bool initialize(const AudioDecoderInitParam&)
// The command block is shorter than its declared size.
void test_e3g8kd7pqvxm2r5h()
{
	const auto chunk = make_chunk(8, {0x01, 0x20, 0x01, 0x00});
	const auto decoder = make_decoder(chunk);
	tester.check(decoder == nullptr);
}

// ==========================================================================

// int decode_frames(float*, int)
void test_t6nz0aw4cufj9bs1()
{
	const auto chunk = make_chunk(4, {0x01, 0x20, 0x01, 0x00});
	const auto decoder = make_decoder(chunk);

	if (decoder == nullptr)
	{
		tester.check(false);
		return;
	}

	constexpr auto max_frames = 1024;
	auto samples = std::vector<float>(max_frames * decoder->get_channel_count());
	const auto decoded_frame_count = decoder->decode_frames(samples.data(), max_frames);
	tester.check(decoded_frame_count > 0);
}

// int decode_frames(float*, int)
// An empty command block decodes nothing, yet rewinding it succeeds. A mixer
// that keeps retrying such a decoder never makes progress.
void test_y2xl5mp8dhqk7v3g()
{
	const auto chunk = make_chunk(0, {});
	const auto decoder = make_decoder(chunk);

	if (decoder == nullptr)
	{
		tester.check(false);
		return;
	}

	constexpr auto max_frames = 1024;
	auto samples = std::vector<float>(max_frames * decoder->get_channel_count());
	const auto first_count = decoder->decode_frames(samples.data(), max_frames);
	const auto is_rewound = decoder->rewind();
	const auto second_count = decoder->decode_frames(samples.data(), max_frames);
	tester.check(first_count == 0 && is_rewound && second_count == 0);
}

// int decode_frames(float*, int)
// Commands without a delay produce no frames either.
void test_u4bs9tr1nwce6zj0()
{
	const auto chunk = make_chunk(8, {0x01, 0x20, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00});
	const auto decoder = make_decoder(chunk);

	if (decoder == nullptr)
	{
		tester.check(false);
		return;
	}

	constexpr auto max_frames = 1024;
	auto samples = std::vector<float>(max_frames * decoder->get_channel_count());
	const auto first_count = decoder->decode_frames(samples.data(), max_frames);
	const auto is_rewound = decoder->rewind();
	const auto second_count = decoder->decode_frames(samples.data(), max_frames);
	tester.check(first_count == 0 && is_rewound && second_count == 0);
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
		tester.register_test("OplMusicDecoder#q7m2vh4nx8ktz3pd", test_q7m2vh4nx8ktz3pd);
		tester.register_test("OplMusicDecoder#w9c5rj1bstn6yl0a", test_w9c5rj1bstn6yl0a);
		tester.register_test("OplMusicDecoder#e3g8kd7pqvxm2r5h", test_e3g8kd7pqvxm2r5h);
	}

	void register_decode_frames()
	{
		tester.register_test("OplMusicDecoder#t6nz0aw4cufj9bs1", test_t6nz0aw4cufj9bs1);
		tester.register_test("OplMusicDecoder#y2xl5mp8dhqk7v3g", test_y2xl5mp8dhqk7v3g);
		tester.register_test("OplMusicDecoder#u4bs9tr1nwce6zj0", test_u4bs9tr1nwce6zj0);
	}
};

auto registrator = Registrator{};

} // namespace
