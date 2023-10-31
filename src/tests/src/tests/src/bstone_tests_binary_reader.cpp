#include "bstone_binary_reader.h"
#include "bstone_static_ro_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// BinaryReader(Stream&)
void test_nayppnn4t19thysw()
{
	auto stream = bstone::StaticRoMemoryStream{};
	auto reader = bstone::BinaryReader{stream};
	tester.check(&reader.get_stream() == &stream);
}

// BinaryReader(const BinaryReader&)
void test_8esp97ufdh2ajljq()
{
	auto stream = bstone::StaticRoMemoryStream{};
	auto reader_1 = bstone::BinaryReader{stream};
	auto reader_2 = reader_1;
	tester.check(&reader_2.get_stream() == &stream);
}

// BinaryReader& operator=(const BinaryReader&)
void test_vzpotnvomakeftsp()
{
	auto stream_1 = bstone::StaticRoMemoryStream{};
	auto reader_1 = bstone::BinaryReader{stream_1};

	auto stream_2 = bstone::StaticRoMemoryStream{};
	auto reader_2 = bstone::BinaryReader{stream_2};
	reader_2 = reader_1;

	tester.check(&reader_2.get_stream() == &stream_1);
}

// ==========================================================================

// Stream& get_stream() const
void test_g9b4peibw9wvcd48()
{
	auto stream = bstone::StaticRoMemoryStream{};
	auto reader = bstone::BinaryReader{stream};
	tester.check(&reader.get_stream() == &stream);
}

// ==========================================================================

// std::int8_t read_s8() const
void test_6uqfld6c9ub3h0mq()
{
	constexpr auto ref_value = std::int8_t{-100};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 1};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_s8();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::uint8_t read_u8() const
void test_ld5gar6n9yp3410a()
{
	constexpr auto ref_value = std::uint8_t{200};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 1};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_u8();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::int16_t read_s16() const
void test_mv90olv9jbv83m1a()
{
	constexpr auto ref_value = std::int16_t{-30'000};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 2};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_s16();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::uint16_t read_u16() const
void test_3o7ghl8gywfvvi69()
{
	constexpr auto ref_value = std::uint16_t{60'000};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 2};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_u16();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::int32_t read_s32() const
void test_wedd7j7e241b9fgw()
{
	constexpr auto ref_value = std::int32_t{-2'000'000'000};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 4};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_s32();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::uint32_t read_u32() const
void test_6bxl1id01qu0a2xu()
{
	constexpr auto ref_value = std::uint32_t{4'000'000'000U};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 4};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_u32();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::int64_t read_s64() const
void test_2nbotcpq6vvekm6b()
{
	constexpr auto ref_value = std::int64_t{-9'000'000'000'000'000'000LL};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 8};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_s64();
	tester.check(value == ref_value);
}

// ==========================================================================

// std::uint64_t read_u64() const
void test_0ik8u8fe2eye55lv()
{
	constexpr auto ref_value = std::uint64_t{18'000'000'000'000'000'000ULL};
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 8};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_u64();
	tester.check(value == ref_value);
}

// ==========================================================================

// float read_b32() const
void test_ogzi2gagfvn4n9u4()
{
	constexpr auto ref_value = 1.23456789F;
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 4};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_b32();
	tester.check(value == ref_value);
}

// ==========================================================================

// double read_b64() const
void test_t0h3ojpbcjsjy0r6()
{
	constexpr auto ref_value = 1.234567890123456789;
	auto stream = bstone::StaticRoMemoryStream{&ref_value, 8};
	auto reader = bstone::BinaryReader{stream};
	const auto value = reader.read_b64();
	tester.check(value == ref_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_binary_reader();
		register_get_stream();
		register_read_s8();
		register_read_u8();
		register_read_s16();
		register_read_u16();
		register_read_s32();
		register_read_u32();
		register_read_s64();
		register_read_u64();
		register_read_b32();
		register_read_b64();
	}

private:
	void register_binary_reader()
	{
		tester.register_test("BinaryReader#nayppnn4t19thysw", test_nayppnn4t19thysw);
		tester.register_test("BinaryReader#8esp97ufdh2ajljq", test_8esp97ufdh2ajljq);
		tester.register_test("BinaryReader#vzpotnvomakeftsp", test_vzpotnvomakeftsp);
	}

	void register_get_stream()
	{
		tester.register_test("BinaryReader#g9b4peibw9wvcd48", test_g9b4peibw9wvcd48);
	}

	void register_read_s8()
	{
		tester.register_test("BinaryReader#6uqfld6c9ub3h0mq", test_6uqfld6c9ub3h0mq);
	}

	void register_read_u8()
	{
		tester.register_test("BinaryReader#ld5gar6n9yp3410a", test_ld5gar6n9yp3410a);
	}

	void register_read_s16()
	{
		tester.register_test("BinaryReader#mv90olv9jbv83m1a", test_mv90olv9jbv83m1a);
	}

	void register_read_u16()
	{
		tester.register_test("BinaryReader#3o7ghl8gywfvvi69", test_3o7ghl8gywfvvi69);
	}

	void register_read_s32()
	{
		tester.register_test("BinaryReader#wedd7j7e241b9fgw", test_wedd7j7e241b9fgw);
	}

	void register_read_u32()
	{
		tester.register_test("BinaryReader#6bxl1id01qu0a2xu", test_6bxl1id01qu0a2xu);
	}

	void register_read_s64()
	{
		tester.register_test("BinaryReader#2nbotcpq6vvekm6b", test_2nbotcpq6vvekm6b);
	}

	void register_read_u64()
	{
		tester.register_test("BinaryReader#0ik8u8fe2eye55lv", test_0ik8u8fe2eye55lv);
	}

	void register_read_b32()
	{
		tester.register_test("BinaryReader#ogzi2gagfvn4n9u4", test_ogzi2gagfvn4n9u4);
	}

	void register_read_b64()
	{
		tester.register_test("BinaryReader#t0h3ojpbcjsjy0r6", test_t0h3ojpbcjsjy0r6);
	}
};

auto registrator = Registrator{};

} // namespace
