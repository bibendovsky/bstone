#include <algorithm>
#include <iterator>

#include "bstone_char_traits.h"
#include "bstone_tester.h"
#include "bstone_utf.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

const char* cbegin(const char* string)
{
	return string;
}

const char* cend(const char* string)
{
	return cbegin(string) + bstone::char_traits::get_size(string);
}

const char16_t* cbegin(const char16_t* string)
{
	return string;
}

const char16_t* cend(const char16_t* string)
{
	return cbegin(string) + bstone::char_traits::get_size(string);
}

// ==========================================================================

// utf::get_u8_to_u16_size(TU8CodeUnitIter, TU8CodeUnitIter)
// All possible code unit sequences of different size.
void test_0lpjv1kodjkztpmf()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf8_code_units = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81";

	const auto dst_utf16_code_unit_count = bstone::utf::get_u8_to_u16_size(
		cbegin(src_utf8_code_units),
		cend(src_utf8_code_units));

	tester.check(dst_utf16_code_unit_count == 5);
}

// utf::get_u8_to_u16_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Invalid sequence.
void test_nnwwoqyg7konqxsl()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xF5\xF6";
		bstone::utf::get_u8_to_u16_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::get_u8_to_u16_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Overlong sequence.
void test_6f3a0e6q2jnlal4x()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xC0\x80";
		bstone::utf::get_u8_to_u16_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::get_u8_to_u16_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Truncated sequence.
void test_7tljfp7cjlz8wnwy()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xF0\x9D";
		bstone::utf::get_u8_to_u16_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// utf::u8_to_u16(TU8CodeUnitIter, TU8CodeUnitIter, TU16CodeUnitIter, TU16CodeUnitIter)
// All possible code unit sequences of different size.
void test_czvy8074i8yhmu27()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf8_code_units = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81";
	constexpr auto src_utf8_code_unit_count = bstone::char_traits::get_size(src_utf8_code_units);
	constexpr auto ref_dst_utf16_code_points = u"\x0023\x00A9\x2116\xD835\xDEC1";
	char16_t dst_utf16_code_units[src_utf8_code_unit_count];

	const auto utf8_to_utf16_result = bstone::utf::u8_to_u16(
		cbegin(src_utf8_code_units),
		cend(src_utf8_code_units),
		std::begin(dst_utf16_code_units),
		std::end(dst_utf16_code_units));

	tester.check(
		utf8_to_utf16_result == dst_utf16_code_units + 5 &&
		std::equal(
			dst_utf16_code_units,
			utf8_to_utf16_result,
			ref_dst_utf16_code_points));
}

// utf::u8_to_u16(TU8CodeUnitIter, TU8CodeUnitIter, TU16CodeUnitIter, TU16CodeUnitIter)
// Invalid sequence.
void test_dnetf2c9m0mjcrcx()
{
	constexpr auto utf8_code_units = "\xF5\xF6";

	auto is_failed = false;

	try
	{
		constexpr auto utf8_code_units_count = bstone::char_traits::get_size(utf8_code_units);
		char16_t utf16_code_units[utf8_code_units_count];

		bstone::utf::u8_to_u16(
			cbegin(utf8_code_units),
			cend(utf8_code_units),
			std::begin(utf16_code_units),
			std::end(utf16_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::u8_to_u16(TU8CodeUnitIter, TU8CodeUnitIter, TU16CodeUnitIter, TU16CodeUnitIter)
// Overlong sequence.
void test_sv51lw9dsztqdmtc()
{
	constexpr auto utf8_code_units = "\xC0\x80";

	auto is_failed = false;

	try
	{
		constexpr auto utf8_code_units_count = bstone::char_traits::get_size(utf8_code_units);
		char16_t utf16_code_units[utf8_code_units_count];

		bstone::utf::u8_to_u16(
			cbegin(utf8_code_units),
			cend(utf8_code_units),
			std::begin(utf16_code_units),
			std::end(utf16_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::u8_to_u16(TU8CodeUnitIter, TU8CodeUnitIter, TU16CodeUnitIter, TU16CodeUnitIter)
// Truncated sequence.
void test_wh5ihznd5ts5gr8o()
{
	constexpr auto utf8_code_units = "\xF0\x9D";

	auto is_failed = false;

	try
	{
		constexpr auto utf8_code_units_count = bstone::char_traits::get_size(utf8_code_units);
		char16_t utf16_code_units[utf8_code_units_count];

		bstone::utf::u8_to_u16(
			cbegin(utf8_code_units),
			cend(utf8_code_units),
			std::begin(utf16_code_units),
			std::end(utf16_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// utf::get_u8_to_u32_size(TU8CodeUnitIter, TU8CodeUnitIter)
// All possible code unit sequences of different size.
void test_4h6ocsp560zzvcua()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf8_code_units = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81";

	const auto dst_utf32_code_unit_count = bstone::utf::get_u8_to_u32_size(
		cbegin(src_utf8_code_units),
		cend(src_utf8_code_units));

	tester.check(dst_utf32_code_unit_count == 4);
}

// utf::get_u8_to_u32_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Overlong sequence.
void test_1bw7cq8tcoo00dt4()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xF5\xF6";
		bstone::utf::get_u8_to_u32_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::get_u8_to_u32_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Truncated sequence.
void test_xrny6fxdr08l8c94()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xC0\x80";
		bstone::utf::get_u8_to_u32_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::get_u8_to_u32_size(TU8CodeUnitIter, TU8CodeUnitIter)
// Invalid sequence.
void test_c2ff9o4knuopgwpw()
{
	auto is_failed = false;

	try
	{
		const auto string = "\xF0\x9D";
		bstone::utf::get_u8_to_u32_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// utf::u8_to_u32(TU8CodeUnitIter, TU8CodeUnitIter, TU32CodeUnitIter, TU32CodeUnitIter)
// All possible code unit sequences of different size.
void test_7uvcri7uc6mi4sf4()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf8_code_units = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81";
	constexpr auto src_utf8_code_unit_count = bstone::char_traits::get_size(src_utf8_code_units);
	constexpr auto ref_dst_utf32_code_points = U"\U00000023\U000000A9\U00002116\U0001D6C1";
	char32_t dst_utf32_code_units[src_utf8_code_unit_count];

	const auto utf8_to_utf32_result = bstone::utf::u8_to_u32(
		cbegin(src_utf8_code_units),
		cend(src_utf8_code_units),
		std::begin(dst_utf32_code_units),
		std::end(dst_utf32_code_units));

	tester.check(
		utf8_to_utf32_result == dst_utf32_code_units + 4 &&
		std::equal(
			dst_utf32_code_units,
			utf8_to_utf32_result,
			ref_dst_utf32_code_points));
}

// utf::u8_to_u32(TU8CodeUnitIter, TU8CodeUnitIter, TU32CodeUnitIter, TU32CodeUnitIter)
// Invalid sequence.
void test_vcbsqol59qb8ax2e()
{
	constexpr auto src_utf8_code_units = "\xF5\xF6";
	constexpr auto src_utf8_code_unit_count = bstone::char_traits::get_size(src_utf8_code_units);
	char32_t dst_utf32_code_units[src_utf8_code_unit_count];

	auto is_failed = false;

	try
	{
		bstone::utf::u8_to_u32(
			cbegin(src_utf8_code_units),
			cend(src_utf8_code_units),
			std::begin(dst_utf32_code_units),
			std::end(dst_utf32_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::u8_to_u32(TU8CodeUnitIter, TU8CodeUnitIter, TU32CodeUnitIter, TU32CodeUnitIter)
// Overlong sequence.
void test_qy3jgipt9m0np06o()
{
	constexpr auto src_utf8_code_units = "\xC0\x80";
	constexpr auto src_utf8_code_unit_count = bstone::char_traits::get_size(src_utf8_code_units);
	char32_t dst_utf32_code_units[src_utf8_code_unit_count];

	auto is_failed = false;

	try
	{
		bstone::utf::u8_to_u32(
			cbegin(src_utf8_code_units),
			cend(src_utf8_code_units),
			std::begin(dst_utf32_code_units),
			std::end(dst_utf32_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::u8_to_u32(TU8CodeUnitIter, TU8CodeUnitIter, TU32CodeUnitIter, TU32CodeUnitIter)
// Truncated sequence.
void test_hyqeztvtdvgp15uz()
{
	constexpr auto src_utf8_code_units = "\xF0\x9D";
	constexpr auto src_utf8_code_unit_count = bstone::char_traits::get_size(src_utf8_code_units);
	char32_t dst_utf32_code_units[src_utf8_code_unit_count];

	auto is_failed = false;

	try
	{
		bstone::utf::u8_to_u32(
			cbegin(src_utf8_code_units),
			cend(src_utf8_code_units),
			std::begin(dst_utf32_code_units),
			std::end(dst_utf32_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// utf::get_u16_to_u8_size(TU16CodeUnitIter, TU16CodeUnitIter)
// All possible code unit sequences of different size.
void test_bam1qo46u38z3tgb()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf16_code_units = u"\x0023\x00A9\x2116\xD835\xDEC1";

	const auto dst_utf8_code_unit_count = bstone::utf::get_u16_to_u8_size(
		cbegin(src_utf16_code_units),
		cend(src_utf16_code_units)
	);

	tester.check(dst_utf8_code_unit_count == 10);
}

// utf::get_u16_to_u8_size(TU16CodeUnitIter, TU16CodeUnitIter)
// Trailing surrogate.
void test_w9zoxxjd77b6wsb1()
{
	auto is_failed = false;

	try
	{
		const auto string = u"\xDCCC\xD888";
		bstone::utf::get_u16_to_u8_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::get_u16_to_u8_size(TU16CodeUnitIter, TU16CodeUnitIter)
// Leading surrogate.
void test_3p57ejsc9ax1zroy()
{
	auto is_failed = false;

	try
	{
		const auto string = u"\xD888";
		bstone::utf::get_u16_to_u8_size(cbegin(string), cend(string));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// utf::u16_to_u8(TU16CodeUnitIter, TU16CodeUnitIter, TU8CodeUnitIter, TU8CodeUnitIter)
// All possible code unit sequences of different size.
void test_3api00kcxm8scvgm()
{
	// Input text: "#©№𝛁"
	// Output code points: "23 A9 2116 1D6C1"
	constexpr auto src_utf16_code_units = u"\x0023\x00A9\x2116\xD835\xDEC1";

	constexpr auto src_utf16_code_unit_count = bstone::char_traits::get_size(src_utf16_code_units);

	constexpr auto ref_dst_utf8_code_points = "\x23\xC2\xA9\xE2\x84\x96\xF0\x9D\x9B\x81";

	constexpr auto max_dst_utf8_code_units = 3 * src_utf16_code_unit_count;
	char dst_utf8_code_units[max_dst_utf8_code_units];

	const auto utf16_to_utf8_result = bstone::utf::u16_to_u8(
		cbegin(src_utf16_code_units),
		cend(src_utf16_code_units),
		std::begin(dst_utf8_code_units),
		std::end(dst_utf8_code_units)
	);

	tester.check(
		utf16_to_utf8_result == dst_utf8_code_units + 10 &&
		std::equal(
			dst_utf8_code_units,
			utf16_to_utf8_result,
			ref_dst_utf8_code_points));
}

// utf::u16_to_u8(TU16CodeUnitIter, TU16CodeUnitIter, TU8CodeUnitIter, TU8CodeUnitIter)
// Trailing surrogate.
void test_orbq31aoq4yzla3w()
{
	constexpr auto src_utf16_code_units = u"\xDCCC\xD888";
	constexpr auto src_utf16_code_unit_count = bstone::char_traits::get_size(src_utf16_code_units);
	constexpr auto max_dst_utf8_code_units = 3 * src_utf16_code_unit_count;
	char dst_utf8_code_units[max_dst_utf8_code_units];

	auto is_failed = false;

	try
	{
		bstone::utf::u16_to_u8(
			cbegin(src_utf16_code_units),
			cend(src_utf16_code_units),
			std::begin(dst_utf8_code_units),
			std::end(dst_utf8_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// utf::u16_to_u8(TU16CodeUnitIter, TU16CodeUnitIter, TU8CodeUnitIter, TU8CodeUnitIter)
// Leading surrogate.
void test_rjm3fdqei0x53jz0()
{
	constexpr auto src_utf16_code_units = u"\xD888";
	constexpr auto src_utf16_code_unit_count = bstone::char_traits::get_size(src_utf16_code_units);
	constexpr auto max_dst_utf8_code_units = 3 * src_utf16_code_unit_count;
	char dst_utf8_code_units[max_dst_utf8_code_units];

	auto is_failed = false;

	try
	{
		bstone::utf::u16_to_u8(
			cbegin(src_utf16_code_units),
			cend(src_utf16_code_units),
			std::begin(dst_utf8_code_units),
			std::end(dst_utf8_code_units));
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_utf_get_utf8_to_utf16_size();
		register_utf_utf8_to_utf16();
		register_utf_get_utf8_to_utf32_size();
		register_utf_utf8_to_utf32();
		register_utf_get_utf16_to_utf8_size();
		register_utf_utf16_to_utf8();
	}

private:
	void register_utf_get_utf8_to_utf16_size()
	{
		tester.register_test("utf::get_u8_to_u16_size#0lpjv1kodjkztpmf", test_0lpjv1kodjkztpmf);
		tester.register_test("utf::get_u8_to_u16_size#nnwwoqyg7konqxsl", test_nnwwoqyg7konqxsl);
		tester.register_test("utf::get_u8_to_u16_size#6f3a0e6q2jnlal4x", test_6f3a0e6q2jnlal4x);
		tester.register_test("utf::get_u8_to_u16_size#7tljfp7cjlz8wnwy", test_7tljfp7cjlz8wnwy);
	}

	void register_utf_utf8_to_utf16()
	{
		tester.register_test("utf::u8_to_u16#czvy8074i8yhmu27", test_czvy8074i8yhmu27);
		tester.register_test("utf::u8_to_u16#dnetf2c9m0mjcrcx", test_dnetf2c9m0mjcrcx);
		tester.register_test("utf::u8_to_u16#sv51lw9dsztqdmtc", test_sv51lw9dsztqdmtc);
		tester.register_test("utf::u8_to_u16#wh5ihznd5ts5gr8o", test_wh5ihznd5ts5gr8o);

	}

	void register_utf_get_utf8_to_utf32_size()
	{
		tester.register_test("utf::get_u8_to_u32_size#4h6ocsp560zzvcua", test_4h6ocsp560zzvcua);
		tester.register_test("utf::get_u8_to_u32_size#1bw7cq8tcoo00dt4", test_1bw7cq8tcoo00dt4);
		tester.register_test("utf::get_u8_to_u32_size#xrny6fxdr08l8c94", test_xrny6fxdr08l8c94);
		tester.register_test("utf::get_u8_to_u32_size#c2ff9o4knuopgwpw", test_c2ff9o4knuopgwpw);
	}

	void register_utf_utf8_to_utf32()
	{
		tester.register_test("utf::u8_to_u32#7uvcri7uc6mi4sf4", test_7uvcri7uc6mi4sf4);
		tester.register_test("utf::u8_to_u32#vcbsqol59qb8ax2e", test_vcbsqol59qb8ax2e);
		tester.register_test("utf::u8_to_u32#qy3jgipt9m0np06o", test_qy3jgipt9m0np06o);
		tester.register_test("utf::u8_to_u32#hyqeztvtdvgp15uz", test_hyqeztvtdvgp15uz);
	}

	void register_utf_get_utf16_to_utf8_size()
	{
		tester.register_test("utf::get_u16_to_u8_size#bam1qo46u38z3tgb", test_bam1qo46u38z3tgb);
		tester.register_test("utf::get_u16_to_u8_size#w9zoxxjd77b6wsb1", test_w9zoxxjd77b6wsb1);
		tester.register_test("utf::get_u16_to_u8_size#3p57ejsc9ax1zroy", test_3p57ejsc9ax1zroy);
	}

	void register_utf_utf16_to_utf8()
	{
		tester.register_test("utf::u16_to_u8#3api00kcxm8scvgm", test_3api00kcxm8scvgm);
		tester.register_test("utf::u16_to_u8#orbq31aoq4yzla3w", test_orbq31aoq4yzla3w);
		tester.register_test("utf::u16_to_u8#rjm3fdqei0x53jz0", test_rjm3fdqei0x53jz0);
	}
};

auto registrator = Registrator{};

} // namespace
