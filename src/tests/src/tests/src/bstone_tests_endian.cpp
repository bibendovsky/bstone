#include <cstdint>

#include "bstone_tester.h"

#include "bstone_endian.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// swap_bytes(T)
// Signed (8 bits).
void test_nq6krovmdvx207cj()
{
	constexpr auto value = static_cast<std::int8_t>(0xAA);
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(value == result);
}

// swap_bytes(T)
// Unsigned (8 bits).
void test_n835bclavbqlbfpv()
{
	constexpr auto value = std::uint8_t{0xAA};
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(value == result);
}

// swap_bytes(T)
// Signed (16 bits).
void test_nsfv4l40huk27d6y()
{
	constexpr auto value = static_cast<std::int16_t>(0xAABB);
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == static_cast<std::int16_t>(0xBBAA));
}

// swap_bytes(T)
// Unsigned (16 bits).
void test_yn51fend8zckr13t()
{
	constexpr auto value = std::uint16_t{0xAABB};
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == 0xBBAA);
}

// swap_bytes(T)
// Signed (32 bits).
void test_rmxiffaahv4op333()
{
	constexpr auto value = static_cast<std::int32_t>(0xAABBCCDD);
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == static_cast<std::int32_t>(0xDDCCBBAA));
}

// swap_bytes(T)
// Unsigned (32 bits).
void test_vkz11uacbewtg48l()
{
	constexpr auto value = std::uint32_t{0xAABBCCDD};
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == 0xDDCCBBAA);
}

// swap_bytes(T)
// Signed (64 bits).
void test_yxbmb2didd4mgfzm()
{
	constexpr auto value = static_cast<std::int64_t>(0xAABBCCDDEEFF1122);
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == static_cast<std::int64_t>(0x2211FFEEDDCCBBAA));
}

// swap_bytes(T)
// Unsigned (64 bits).
void test_pszoogly5ksib4b6()
{
	constexpr auto value = std::uint64_t{0xAABBCCDDEEFF1122};
	constexpr auto result = bstone::endian::swap_bytes(value);
	tester.check(result == 0x2211FFEEDDCCBBAA);
}

// ==========================================================================

void test_g7a3yy0uvcn10b0d()
{
	constexpr auto result = bstone::endian::to_little(std::uint16_t{0xAABBU});

	tester.check(result ==
#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
		0xAABBU
#elif BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
		0xBBAAU
#endif
	);
}

void test_1izea90wddx37cq6()
{
	constexpr auto result = bstone::endian::to_big(std::uint16_t{0xAABBU});

	tester.check(result ==
#if BSTONE_ENDIAN == BSTONE_LITTLE_ENDIAN
		0xBBAAU
#elif BSTONE_ENDIAN == BSTONE_BIG_ENDIAN
		0xAABBU
#endif
	);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_swap_bytes();
		register_to_little();
		register_to_big();
	}

private:
	void register_swap_bytes()
	{
		tester.register_test("swap_bytes#nq6krovmdvx207cj", test_nq6krovmdvx207cj);
		tester.register_test("swap_bytes#n835bclavbqlbfpv", test_n835bclavbqlbfpv);

		tester.register_test("swap_bytes#nsfv4l40huk27d6y", test_nsfv4l40huk27d6y);
		tester.register_test("swap_bytes#yn51fend8zckr13t", test_yn51fend8zckr13t);

		tester.register_test("swap_bytes#rmxiffaahv4op333", test_rmxiffaahv4op333);
		tester.register_test("swap_bytes#vkz11uacbewtg48l", test_vkz11uacbewtg48l);

		tester.register_test("swap_bytes#yxbmb2didd4mgfzm", test_yxbmb2didd4mgfzm);
		tester.register_test("swap_bytes#pszoogly5ksib4b6", test_pszoogly5ksib4b6);
	}

	void register_to_little()
	{
		tester.register_test("to_little#g7a3yy0uvcn10b0d", test_g7a3yy0uvcn10b0d);
	}

	void register_to_big()
	{
		tester.register_test("to_big#1izea90wddx37cq6", test_1izea90wddx37cq6);
	}
};

auto registrator = Registrator{};

} // namespace
