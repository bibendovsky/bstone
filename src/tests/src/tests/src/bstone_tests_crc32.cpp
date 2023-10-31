#include <string>

#include "bstone_crc32.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

std::intptr_t length(const char* string)
{
	return static_cast<std::intptr_t>(std::char_traits<char>::length(string));
}

// ==========================================================================

// Crc32()
void test_vb62wkvrvp1o2kh2()
{
	const auto crc32 = bstone::Crc32{};
	tester.check(crc32.get_value() == 0);
}

// Crc32(const Crc32&)
void test_2kil0trb0ij6ika5()
{
	constexpr auto data = "1";

	auto crc32_1 = bstone::Crc32{};
	crc32_1.update(data, length(data));
	const auto value_1 = crc32_1.get_value();

	auto crc32_2 = crc32_1;
	const auto value_2 = crc32_2.get_value();

	tester.check(value_1 == value_2);
}

// Crc32& operator=(const Crc32&)
void test_k7fbenfapzozmb5x()
{
	constexpr auto data = "1";

	auto crc32_1 = bstone::Crc32{};
	crc32_1.update(data, length(data));
	const auto value_1 = crc32_1.get_value();

	auto crc32_2 = bstone::Crc32{};
	crc32_2 = crc32_1;
	const auto value_2 = crc32_2.get_value();

	tester.check(value_1 == value_2);
}

// ==========================================================================

// Crc32Value get_value() const noexcept
void test_2drajpurv3igoumo()
{
	const auto crc32 = bstone::Crc32{};
	tester.check(crc32.get_value() == 0);
}

// ==========================================================================

// void reset() noexcept
void test_kujcosz33c9owex3()
{
	auto crc32 = bstone::Crc32{};
	constexpr auto data = "1";
	crc32.update(data, length(data));
	const auto value_1 = crc32.get_value();

	crc32.reset();
	const auto value_2 = crc32.get_value();

	tester.check(value_1 != 0 && value_2 == 0);
}

// ==========================================================================

// void update(const void* data, std::intptr_t size)
// "a"
void test_jhujyqaohj9ry9s8()
{
	constexpr auto ref_value = 0xE8B7BE43;
	const auto ref_data = "a";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// void update(const void* data, std::intptr_t size)
// "abc"
void test_n0nxkm8jrb8l037n()
{
	constexpr auto ref_value = 0x352441C2;
	const auto ref_data = "abc";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// void update(const void* data, std::intptr_t size)
// "message digest"
void test_4xzs0ogisj32kooi()
{
	constexpr auto ref_value = 0x20159D7F;
	const auto ref_data = "message digest";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// void update(const void* data, std::intptr_t size)
// "abcdefghijklmnopqrstuvwxyz"
void test_smnc6r7yxu6gludi()
{
	constexpr auto ref_value = 0x4C2750BD;
	const auto ref_data = "abcdefghijklmnopqrstuvwxyz";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// void update(const void* data, std::intptr_t size)
// "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
void test_w90o9fvmsx7gmz3p()
{
	constexpr auto ref_value = 0x1FC2E6D2;
	const auto ref_data = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// void update(const void* data, std::intptr_t size)
// "12345678901234567890123456789012345678901234567890123456789012345678901234567890"
void test_0ob9y15z8z0ll2a6()
{
	constexpr auto ref_value = 0x7CA94A72;
	const auto ref_data = "12345678901234567890123456789012345678901234567890123456789012345678901234567890";
	auto crc32 = bstone::Crc32{};
	crc32.update(ref_data, length(ref_data));
	const auto value = crc32.get_value();
	tester.check(value == ref_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_crc32();
		register_get_value();
		register_reset();
		register_update();
	}

private:
	void register_crc32()
	{
		tester.register_test("Crc32#vb62wkvrvp1o2kh2", test_vb62wkvrvp1o2kh2);
		tester.register_test("Crc32#2kil0trb0ij6ika5", test_2kil0trb0ij6ika5);
		tester.register_test("Crc32#k7fbenfapzozmb5x", test_k7fbenfapzozmb5x);
	}

	void register_get_value()
	{
		tester.register_test("Crc32#2drajpurv3igoumo", test_2drajpurv3igoumo);
	}

	void register_reset()
	{
		tester.register_test("Crc32#kujcosz33c9owex3", test_kujcosz33c9owex3);
	}

	void register_update()
	{
		tester.register_test("Crc32#jhujyqaohj9ry9s8", test_jhujyqaohj9ry9s8);
		tester.register_test("Crc32#n0nxkm8jrb8l037n", test_n0nxkm8jrb8l037n);
		tester.register_test("Crc32#4xzs0ogisj32kooi", test_4xzs0ogisj32kooi);
		tester.register_test("Crc32#smnc6r7yxu6gludi", test_smnc6r7yxu6gludi);
		tester.register_test("Crc32#w90o9fvmsx7gmz3p", test_w90o9fvmsx7gmz3p);
		tester.register_test("Crc32#0ob9y15z8z0ll2a6", test_0ob9y15z8z0ll2a6);
	}
};

auto registrator = Registrator{};

} // namespace
