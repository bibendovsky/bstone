#include <cstdint>

#include "bstone_tester.h"

#include "bstone_endian.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// read_s16_le
void test_rmxiffaahv4op333()
{
	const std::uint8_t bytes[2] = {0xBB, 0xAA};
	const std::int16_t value = bstone::endian::read_s16_le(bytes);
	tester.check(value == static_cast<std::int16_t>(0xAABB));
}

// read_u16_le
void test_vkz11uacbewtg48l()
{
	const std::uint8_t bytes[2] = {0xBB, 0xAA};
	const std::uint16_t value = bstone::endian::read_u16_le(bytes);
	tester.check(value == 0xAABB);
}

// read_s32_le
void test_yn51fend8zckr13t()
{
	const std::uint8_t bytes[4] = {0xDD, 0xCC, 0xBB, 0xAA};
	const std::int32_t value = bstone::endian::read_s32_le(bytes);
	tester.check(value == static_cast<std::int32_t>(0xAABBCCDDU));
}

// read_u32_le
void test_nsfv4l40huk27d6y()
{
	const std::uint8_t bytes[4] = {0xDD, 0xCC, 0xBB, 0xAA};
	const std::uint32_t value = bstone::endian::read_u32_le(bytes);
	tester.check(value == 0xAABBCCDDU);
}

// ==========================================================================

// write_s16_le
void test_nq6krovmdvx207cj()
{
	std::uint8_t bytes[2];
	bstone::endian::write_s16_le(static_cast<std::int16_t>(0xAABB), bytes);
	tester.check(bytes[0] == 0xBB && bytes[1] == 0xAA);
}

// write_u16_le
void test_g7a3yy0uvcn10b0d()
{
	std::uint8_t bytes[2];
	bstone::endian::write_u16_le(0xAABB, bytes);
	tester.check(bytes[0] == 0xBB && bytes[1] == 0xAA);
}

// write_s32_le
void test_n835bclavbqlbfpv()
{
	std::uint8_t bytes[4];
	bstone::endian::write_s32_le(static_cast<std::int32_t>(0xAABBCCDDU), bytes);
	tester.check(bytes[0] == 0xDD && bytes[1] == 0xCC && bytes[2] == 0xBB && bytes[3] == 0xAA);
}

// write_u32_le
void test_1izea90wddx37cq6()
{
	std::uint8_t bytes[4];
	bstone::endian::write_u32_le(0xAABBCCDDU, bytes);
	tester.check(bytes[0] == 0xDD && bytes[1] == 0xCC && bytes[2] == 0xBB && bytes[3] == 0xAA);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_read();
		register_write();
	}

private:
	void register_read()
	{
		tester.register_test("read_s16_le#rmxiffaahv4op333", test_rmxiffaahv4op333);
		tester.register_test("read_u16_le#vkz11uacbewtg48l", test_vkz11uacbewtg48l);
		tester.register_test("read_s32_le#yn51fend8zckr13t", test_yn51fend8zckr13t);
		tester.register_test("read_u32_le#nsfv4l40huk27d6y", test_nsfv4l40huk27d6y);
	}

	void register_write()
	{
		tester.register_test("write_s16_le#nq6krovmdvx207cj", test_nq6krovmdvx207cj);
		tester.register_test("write_u16_le#g7a3yy0uvcn10b0d", test_g7a3yy0uvcn10b0d);
		tester.register_test("write_s32_le#n835bclavbqlbfpv", test_n835bclavbqlbfpv);
		tester.register_test("write_u32_le#1izea90wddx37cq6", test_1izea90wddx37cq6);
	}
};

auto registrator = Registrator{};

} // namespace
