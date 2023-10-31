#include "bstone_binary_writer.h"
#include "bstone_static_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// BinaryWriter(Stream&)
void test_gc00uuoo89wp7cqp()
{
	auto stream = bstone::StaticMemoryStream{};
	auto writer = bstone::BinaryWriter{stream};
	tester.check(&writer.get_stream() == &stream);
}

// BinaryWriter(const BinaryWriter&)
void test_fg2dqopfrcx4fnjv()
{
	auto stream = bstone::StaticMemoryStream{};
	auto writer_1 = bstone::BinaryWriter{stream};
	auto writer_2 = writer_1;
	tester.check(&writer_2.get_stream() == &stream);
}

// BinaryWriter& operator=(const BinaryWriter&)
void test_s121259ci3w0a25n()
{
	auto stream_1 = bstone::StaticMemoryStream{};
	auto writer_1 = bstone::BinaryWriter{stream_1};

	auto stream_2 = bstone::StaticMemoryStream{};
	auto writer_2 = bstone::BinaryWriter{stream_2};
	writer_2 = writer_1;

	tester.check(&writer_2.get_stream() == &stream_1);
}

// ==========================================================================

// Stream& get_stream() const
void test_ax1hkhjhko0irl0z()
{
	auto stream = bstone::StaticMemoryStream{};
	auto writer = bstone::BinaryWriter{stream};
	tester.check(&writer.get_stream() == &stream);
}

// ==========================================================================

// void write_s8(std::int8_t) const
void test_eivkgxx3pcjeykj4()
{
	constexpr auto ref_value = std::int8_t{-100};
	auto buffer = std::int8_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 1};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_s8(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_u8(std::uint8_t) const
void test_2irmjqej1gnff783()
{
	constexpr auto ref_value = std::uint8_t{200};
	auto buffer = std::uint8_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 1};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_u8(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_s16(std::int16_t) const
void test_le8papnus9hpeuoz()
{
	constexpr auto ref_value = std::int16_t{-30'000};
	auto buffer = std::int16_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 2};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_s16(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_u16(std::uint16_t) const
void test_pey2mss28ckf2h7v()
{
	constexpr auto ref_value = std::uint16_t{60'000};
	auto buffer = std::uint16_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 2};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_u16(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_s32(std::int32_t) const
void test_o8xq1rjptbzzwcm1()
{
	constexpr auto ref_value = std::int32_t{-2'000'000'000};
	auto buffer = std::int32_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 4};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_s32(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_u32(std::uint32_t) const
void test_o2tw6pkurf2uwxr9()
{
	constexpr auto ref_value = std::uint32_t{4'000'000'000U};
	auto buffer = std::uint32_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 4};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_u32(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_s64(std::int64_t) const
void test_bhh0guk7dzyol9zr()
{
	constexpr auto ref_value = std::int64_t{-9'000'000'000'000'000'000LL};
	auto buffer = std::int64_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 8};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_s64(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_u64(std::uint64_t) const
void test_8lu9fboziinf1k0x()
{
	constexpr auto ref_value = std::uint64_t{18'000'000'000'000'000'000ULL};
	auto buffer = std::uint64_t{};
	auto stream = bstone::StaticMemoryStream{&buffer, 8};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_u64(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_b32(float) const
void test_jujz4d4n2wgnoh2z()
{
	constexpr auto ref_value = 1.23456789F;
	auto buffer = float{};
	auto stream = bstone::StaticMemoryStream{&buffer, 4};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_b32(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

// void write_b64(double) const
void test_kpsvdnret2881ijx()
{
	constexpr auto ref_value = 1.234567890123456789;
	auto buffer = double{};
	auto stream = bstone::StaticMemoryStream{&buffer, 8};
	auto writer = bstone::BinaryWriter{stream};
	writer.write_b64(ref_value);
	tester.check(buffer == ref_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_binary_reader();
		register_get_stream();
		register_write_s8();
		register_write_u8();
		register_write_s16();
		register_write_u16();
		register_write_s32();
		register_write_u32();
		register_write_s64();
		register_write_u64();
		register_write_b32();
		register_write_b64();
	}

private:
	void register_binary_reader()
	{
		tester.register_test("BinaryWriter#gc00uuoo89wp7cqp", test_gc00uuoo89wp7cqp);
		tester.register_test("BinaryWriter#fg2dqopfrcx4fnjv", test_fg2dqopfrcx4fnjv);
		tester.register_test("BinaryWriter#s121259ci3w0a25n", test_s121259ci3w0a25n);
	}

	void register_get_stream()
	{
		tester.register_test("BinaryWriter#ax1hkhjhko0irl0z", test_ax1hkhjhko0irl0z);
	}

	void register_write_s8()
	{
		tester.register_test("BinaryWriter#eivkgxx3pcjeykj4", test_eivkgxx3pcjeykj4);
	}

	void register_write_u8()
	{
		tester.register_test("BinaryWriter#2irmjqej1gnff783", test_2irmjqej1gnff783);
	}

	void register_write_s16()
	{
		tester.register_test("BinaryWriter#le8papnus9hpeuoz", test_le8papnus9hpeuoz);
	}

	void register_write_u16()
	{
		tester.register_test("BinaryWriter#pey2mss28ckf2h7v", test_pey2mss28ckf2h7v);
	}

	void register_write_s32()
	{
		tester.register_test("BinaryWriter#o8xq1rjptbzzwcm1", test_o8xq1rjptbzzwcm1);
	}

	void register_write_u32()
	{
		tester.register_test("BinaryWriter#o2tw6pkurf2uwxr9", test_o2tw6pkurf2uwxr9);
	}

	void register_write_s64()
	{
		tester.register_test("BinaryWriter#bhh0guk7dzyol9zr", test_bhh0guk7dzyol9zr);
	}

	void register_write_u64()
	{
		tester.register_test("BinaryWriter#8lu9fboziinf1k0x", test_8lu9fboziinf1k0x);
	}

	void register_write_b32()
	{
		tester.register_test("BinaryWriter#jujz4d4n2wgnoh2z", test_jujz4d4n2wgnoh2z);
	}

	void register_write_b64()
	{
		tester.register_test("BinaryWriter#kpsvdnret2881ijx", test_kpsvdnret2881ijx);
	}
};

auto registrator = Registrator{};

} // namespace
