#include <cstdint>

#include <limits>

#include "bstone_memory_binary_reader.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

constexpr std::uint8_t test_data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

// ==========================================================================

// MemoryBinaryReader()
void test_8z214ykyxfxr14b3()
{
	const auto reader = bstone::MemoryBinaryReader{};
	const auto is_valid = reader.get_size() == 0 && !reader.can_read_x8();
	tester.check(is_valid);
}

// MemoryBinaryReader(const void*, int)
void test_djr24u31gl1diex9()
{
	const auto reader = bstone::MemoryBinaryReader{test_data, 8};
	const auto is_valid = reader.get_size() == 8 && reader.get_current_data() == test_data;
	tester.check(is_valid);
}

// ==========================================================================

// void set_position(int)
void test_ta7q1jby7efgnpx4()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.set_position(6);
	const auto is_valid = reader.read_u16_le() == 0x8877U;
	tester.check(is_valid);
}

// void set_position(int)
// Out of range positions are clamped.
void test_vbk02bh4q3qfadk4()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.set_position(1000);
	const auto is_at_end = !reader.can_read_x8();
	reader.set_position(-1000);
	const auto is_at_begin = reader.read_u8() == 0x11U;
	tester.check(is_at_end && is_at_begin);
}

// ==========================================================================

// void skip(int)
void test_ybwmn0y31zt4nst5()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.skip(4);
	reader.skip(-2);
	const auto is_valid = reader.read_u8() == 0x33U;
	tester.check(is_valid);
}

// ==========================================================================

// bool can_read_n(int) const
void test_fy8fsvsdci71tmix()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	const auto is_all_readable = reader.can_read_n(8) && !reader.can_read_n(9);
	reader.skip(5);
	const auto is_rest_readable = reader.can_read_n(3) && !reader.can_read_n(4);
	tester.check(is_all_readable && is_rest_readable);
}

// bool can_read_n(int) const
// A negative count vouches for nothing.
void test_okqm1qess2tphyec()
{
	const auto reader = bstone::MemoryBinaryReader{test_data, 8};

	const auto is_valid =
		!reader.can_read_n(-1) &&
		!reader.can_read_n(std::numeric_limits<int>::min());

	tester.check(is_valid);
}

// bool can_read_n(int) const
// Zero octets are always readable.
void test_p837r8095vzlzeov()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.set_position(8);
	const auto is_valid = reader.can_read_n(0);
	tester.check(is_valid);
}

// ==========================================================================

// bool can_read_x8() const
// bool can_read_x16() const
// bool can_read_x32() const
void test_kwqfnzljiljigmxm()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.set_position(5);
	const auto is_valid = reader.can_read_x8() && !reader.can_read_x32();
	reader.set_position(6);
	const auto is_still_valid = reader.can_read_x16() && !reader.can_read_x32();
	tester.check(is_valid && is_still_valid);
}

// ==========================================================================

// std::int8_t read_s8()
// std::uint8_t read_u8()
void test_guyqluiqpau0uzug()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	const auto u8 = reader.read_u8();
	reader.set_position(7);
	const auto s8 = reader.read_s8();
	const auto is_valid = u8 == 0x11U && s8 == -120;
	tester.check(is_valid);
}

// ==========================================================================

// std::int16_t read_s16_le()
// std::uint16_t read_u16_le()
void test_sc33hg54x0sgyyyi()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	const auto u16 = reader.read_u16_le();
	reader.set_position(6);
	const auto s16 = reader.read_s16_le();
	const auto is_valid = u16 == 0x2211U && s16 == -30601;
	tester.check(is_valid);
}

// ==========================================================================

// std::int32_t read_s32_le()
// std::uint32_t read_u32_le()
void test_r8ynd62wn6h544nr()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	const auto u32 = reader.read_u32_le();
	const auto s32 = reader.read_s32_le();
	const auto is_valid = u32 == 0x44332211U && s32 == -2005440939;
	tester.check(is_valid);
}

// ==========================================================================

// void read(void*, int)
void test_dyeqho9i94ssnjdo()
{
	auto reader = bstone::MemoryBinaryReader{test_data, 8};
	reader.skip(4);
	std::uint8_t buffer[4] = {};
	reader.read(buffer, 4);

	const auto is_valid =
		buffer[0] == 0x55U &&
		buffer[1] == 0x66U &&
		buffer[2] == 0x77U &&
		buffer[3] == 0x88U;

	tester.check(is_valid);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_set_position();
		register_skip();
		register_can_read_n();
		register_can_read_x();
		register_read_8();
		register_read_16();
		register_read_32();
		register_read();
	}

private:
	void register_ctor()
	{
		tester.register_test("MemoryBinaryReader#8z214ykyxfxr14b3", test_8z214ykyxfxr14b3);
		tester.register_test("MemoryBinaryReader#djr24u31gl1diex9", test_djr24u31gl1diex9);
	}

	void register_set_position()
	{
		tester.register_test("MemoryBinaryReader#ta7q1jby7efgnpx4", test_ta7q1jby7efgnpx4);
		tester.register_test("MemoryBinaryReader#vbk02bh4q3qfadk4", test_vbk02bh4q3qfadk4);
	}

	void register_skip()
	{
		tester.register_test("MemoryBinaryReader#ybwmn0y31zt4nst5", test_ybwmn0y31zt4nst5);
	}

	void register_can_read_n()
	{
		tester.register_test("MemoryBinaryReader#fy8fsvsdci71tmix", test_fy8fsvsdci71tmix);
		tester.register_test("MemoryBinaryReader#okqm1qess2tphyec", test_okqm1qess2tphyec);
		tester.register_test("MemoryBinaryReader#p837r8095vzlzeov", test_p837r8095vzlzeov);
	}

	void register_can_read_x()
	{
		tester.register_test("MemoryBinaryReader#kwqfnzljiljigmxm", test_kwqfnzljiljigmxm);
	}

	void register_read_8()
	{
		tester.register_test("MemoryBinaryReader#guyqluiqpau0uzug", test_guyqluiqpau0uzug);
	}

	void register_read_16()
	{
		tester.register_test("MemoryBinaryReader#sc33hg54x0sgyyyi", test_sc33hg54x0sgyyyi);
	}

	void register_read_32()
	{
		tester.register_test("MemoryBinaryReader#r8ynd62wn6h544nr", test_r8ynd62wn6h544nr);
	}

	void register_read()
	{
		tester.register_test("MemoryBinaryReader#dyeqho9i94ssnjdo", test_dyeqho9i94ssnjdo);
	}
};

auto registrator = Registrator{};

} // namespace
