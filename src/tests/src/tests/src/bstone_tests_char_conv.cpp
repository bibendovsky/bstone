#include <cstdint>

#include <algorithm>
#include <exception>
#include <numeric>

#include "bstone_tester.h"

#include "bstone_char_conv.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// hex_char_to_nibble(TNibble)
// Valid.
// TNibble: unsigned int
// TChar: int
void test_786367ma88ppf63d()
{
	using TNibble = unsigned int;
	TNibble reference_nibbles[16];
	std::iota(reference_nibbles, reference_nibbles + 16, TNibble{});

	using TChar = int;
	constexpr TChar reference_chars[16] =
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	auto is_succeeded = true;

	for (auto i = 0; i < 16; ++i)
	{
		const auto ch = bstone::nibble_to_hex_char<TChar, TNibble>(reference_nibbles[i]);
		is_succeeded &= ch == reference_chars[i];
	}

	tester.check(is_succeeded);
}

// nibble_to_hex_char(TNibble)
// Invalid.
// TNibble: unsigned int
// TChar: int
void test_1kxpwivq3m8uixis()
{
	using TNibble = unsigned int;
	using TChar = int;

	auto is_failed = false;

	try
	{
		bstone::nibble_to_hex_char<TChar, TNibble>(100U);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// hex_char_to_nibble(TChar)
// Valid.
// TNibble: unsigned int
// TChar: int
void test_thc5mvg8pjtzfmvv()
{
	using TChar = int;
	constexpr TChar reference_chars[22] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'b', 'c', 'd', 'e', 'f',
		'A', 'B', 'C', 'D', 'E', 'F'
	};

	using TNibble = unsigned int;
	constexpr TNibble reference_nibbles[22] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10, 11, 12, 13, 14, 15,
		10, 11, 12, 13, 14, 15
	};

	auto is_succeeded = true;

	for (auto i = 0; i < 22; ++i)
	{
		const auto nibble = bstone::hex_char_to_nibble<TNibble, TChar>(reference_chars[i]);
		is_succeeded &= nibble == reference_nibbles[i];
	}

	tester.check(is_succeeded);
}

// hex_char_to_nibble(TChar)
// Invalid.
// TNibble: unsigned int
// TChar: int
void test_c47zq4hnypy3m18i()
{
	using TChar = int;
	using TNibble = unsigned int;

	auto is_failed = false;

	try
	{
		bstone::hex_char_to_nibble<TNibble, TChar>('g');
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
void test_n1b6jxr93yx60dxm()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[22] = {
		'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
		'a', 'A', 'b', 'B', 'c', 'C', 'd', 'D', 'e', 'E', 'f', 'F'
	};

	constexpr TByte reference_bytes[11] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
	TByte bytes[12];

	const auto bytes_last = bstone::hex_chars_to_bytes(reference_chars, reference_chars + 22, bytes, bytes + 12);

	tester.check(
		bytes_last == bytes + 11 &&
		std::equal(reference_bytes, reference_bytes + 11, bytes));
}

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
// Negative char count.
void test_kzloe3bvve6i0yzw()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[2] = {'0', '1'};
	TByte bytes[1];

	auto is_failed = false;

	try
	{
		bstone::hex_chars_to_bytes(reference_chars + 1, reference_chars, bytes, bytes + 1);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
// Odd char count.
void test_zm2b65au561avo68()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[2] = {'0', '1'};
	TByte bytes[1];

	auto is_failed = false;

	try
	{
		bstone::hex_chars_to_bytes(reference_chars, reference_chars + 1, bytes, bytes + 1);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
// Negative byte count.
void test_nxvdm1h98gzatxzo()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[2] = {'0', '1'};
	TByte bytes[1];

	auto is_failed = false;

	try
	{
		bstone::hex_chars_to_bytes(reference_chars, reference_chars + 1, bytes + 1, bytes);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
// Byte buffer too small.
void test_b9cjw5rk6b7jrckg()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[2] = {'0', '1'};
	TByte bytes[1];

	auto is_failed = false;

	try
	{
		bstone::hex_chars_to_bytes(reference_chars, reference_chars + 2, bytes, bytes);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// hex_chars_to_bytes(const TChar*, const TChar*, TByte*, TByte*)
// Zero char count.
void test_tfs03rluhhsouvil()
{
	using TChar = int;
	using TByte = unsigned char;

	constexpr TChar reference_chars[2] = {'0', '1'};
	TByte bytes[1];

	const auto bytes_last = bstone::hex_chars_to_bytes(reference_chars, reference_chars, bytes, bytes);
	tester.check(bytes_last == bytes);
}

// ==========================================================================

// bytes_to_hex_chars(const TByte*, const TByte*, TChar*, TChar*)
void test_8qxf14j67dqhycp7()
{
	using TByte = unsigned char;
	using TChar = int;

	constexpr TByte reference_bytes[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};

	constexpr TChar reference_chars[16] =
		{'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

	TChar chars[18];

	const auto chars_last = bstone::bytes_to_hex_chars(reference_bytes, reference_bytes + 8, chars, chars + 18);

	tester.check(
		chars_last == chars + 16 &&
		std::equal(reference_chars, reference_chars + 16, chars));
}

// bytes_to_hex_chars(const TByte*, const TByte*, TChar*, TChar*)
// Zero byte count.
void test_qej30gonkno07fhs()
{
	using TByte = unsigned char;
	using TChar = int;

	TByte bytes[1] = {};
	TChar chars[1] = {};

	const auto chars_last = bstone::bytes_to_hex_chars(bytes, bytes, chars, chars + 1);
	tester.check(chars_last == chars);
}

// bytes_to_hex_chars(const TByte*, const TByte*, TChar*, TChar*)
// Negative byte count.
void test_1ghesobjlixkwtjy()
{
	using TByte = unsigned char;
	using TChar = int;

	TByte bytes[1] = {};
	TChar chars[2] = {};

	auto is_failed = false;

	try
	{
		bstone::bytes_to_hex_chars(bytes + 1, bytes, chars, chars + 2);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// bytes_to_hex_chars(const TByte*, const TByte*, TChar*, TChar*)
// Negative char count.
void test_hfno9tjnffo1bciv()
{
	using TByte = unsigned char;
	using TChar = int;

	TByte bytes[1] = {};
	TChar chars[2] = {};

	auto is_failed = false;

	try
	{
		bstone::bytes_to_hex_chars(bytes, bytes + 1, chars + 2, chars);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// bytes_to_hex_chars(const TByte*, const TByte*, TChar*, TChar*)
// Char buffer too small.
void test_l0rlgeb9nnayge7e()
{
	using TByte = unsigned char;
	using TChar = int;

	TByte bytes[1] = {};
	TChar chars[2] = {};

	auto is_failed = false;

	try
	{
		bstone::bytes_to_hex_chars(bytes, bytes + 1, chars, chars + 1);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// to_chars(TValue, TChar*, TChar*, int)
// Octal negative value.
void test_6bsk5jbj8h32j51u()
{
	constexpr auto value = std::int32_t{-2'147'483'647 - 1};
	constexpr char reference_chars[12] = {'-', '2', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
	char chars[13] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 13, 8);

	tester.check(
		chars_last == chars + 12 &&
		std::equal(reference_chars, reference_chars + 12, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Decimal negative value.
void test_w9v94y8ylfdw0fn6()
{
	constexpr auto value = std::int32_t{-2'147'483'647 - 1};
	constexpr char reference_chars[11] = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
	char chars[12] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 12);

	tester.check(
		chars_last == chars + 11 &&
		std::equal(reference_chars, reference_chars + 11, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Hex negative value.
void test_94vh40w9bhfiisuq()
{
	constexpr auto value = std::int32_t{-2'147'483'647 - 1};
	constexpr char reference_chars[9] = {'-', '8', '0', '0', '0', '0', '0', '0', '0'};
	char chars[10] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 10, 16);

	tester.check(
		chars_last == chars + 9 &&
		std::equal(reference_chars, reference_chars + 9, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Octal positive value.
void test_dvtkoajkruya2g23()
{
	constexpr auto value = std::int32_t{2'147'483'647};
	constexpr char reference_chars[11] = {'1', '7', '7', '7', '7', '7', '7', '7', '7', '7', '7'};
	char chars[12] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 12, 8);

	tester.check(
		chars_last == chars + 11 &&
		std::equal(reference_chars, reference_chars + 11, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Decimal positive value.
void test_gci8hnnnfrzce1ps()
{
	constexpr auto value = std::int32_t{2'147'483'647};
	constexpr char reference_chars[10] = {'2', '1', '4', '7', '4', '8', '3', '6', '4', '7'};
	char chars[11] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 11);

	tester.check(
		chars_last == chars + 10 &&
		std::equal(reference_chars, reference_chars + 10, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Hex positive value.
void test_k5dma16zcskflljr()
{
	constexpr auto value = std::int32_t{2'147'483'647};
	constexpr char reference_chars[8] = {'7', 'f', 'f', 'f', 'f', 'f', 'f', 'f'};
	char chars[9] = {};
	const auto chars_last = bstone::to_chars(value, chars, chars + 9, 16);

	tester.check(
		chars_last == chars + 8 &&
		std::equal(reference_chars, reference_chars + 8, chars));
}

// to_chars(TValue, TChar*, TChar*, int)
// Base out of range.
void test_yu95gjn7r8e549ki()
{
	char chars[1] = {};
	auto is_failed_1 = false;
	auto is_failed_2 = false;

	try
	{
		bstone::to_chars(0, chars, chars + 1, bstone::char_conv_min_base - 1);
	}
	catch (const std::exception&)
	{
		is_failed_1 = true;
	}

	try
	{
		bstone::to_chars(0, chars, chars + 1, bstone::char_conv_max_base + 1);
	}
	catch (const std::exception&)
	{
		is_failed_2 = true;
	}

	tester.check(is_failed_1 && is_failed_2);
}

// to_chars(TValue, TChar*, TChar*, int)
// Buffer too small.
void test_hlb3rbu9n9ru7j0p()
{
	char chars[1] = {};
	auto is_failed_1 = false;
	auto is_failed_2 = false;

	try
	{
		bstone::to_chars(0, chars, chars);
	}
	catch (const std::exception&)
	{
		is_failed_1 = true;
	}

	try
	{
		bstone::to_chars(10, chars, chars + 1);
	}
	catch (const std::exception&)
	{
		is_failed_2 = true;
	}

	tester.check(is_failed_1 && is_failed_2);
}

// ==========================================================================

// from_chars(const TChar*, const TChar*, TValue&, int)
// Octal negative number; explicit base.
void test_2l8uvpq5jwsq1mey()
{
	constexpr std::int32_t reference_value = -2'147'483'647 - 1;
	constexpr char reference_chars[12] = {'-', '2', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 12, value, 8);
	tester.check(chars_last == reference_chars + 12 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Octal negative number; implicit base.
void test_6g2pgwz3xscy7a0s()
{
	constexpr std::int32_t reference_value = -2'147'483'647 - 1;
	constexpr char reference_chars[13] = {'-', '0', '2', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 13, value, 0);
	tester.check(chars_last == reference_chars + 13 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Octal number; implicit base.
void test_xhrlosdet2rdbpnv()
{
	constexpr std::int32_t reference_value = 1'402'433'619;
	constexpr char reference_chars[12] = {'0', '1', '2', '3', '4', '5', '6', '7', '0', '1', '2', '3'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 12, value, 0);
	tester.check(chars_last == reference_chars + 12 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Decimal negative number; explicit base.
void test_5aohfvv83ivwlzq0()
{
	constexpr std::int32_t reference_value = -2'147'483'647 - 1;
	constexpr char reference_chars[11] = {'-', '2', '1', '4', '7', '4', '8', '3', '6', '4', '8'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 11, value);
	tester.check(chars_last == reference_chars + 11 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Decimal negative number; implicit base.
void test_ah55jmdmze5tbq7e()
{
	constexpr std::int32_t reference_value = 1'234'567'890;
	constexpr char reference_chars[10] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 10, value, 0);
	tester.check(chars_last == reference_chars + 10 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Hex negative number; explicit base.
void test_clkb1xo609m44vyu()
{
	constexpr std::int32_t reference_value = -2'058'993'460;
	constexpr char reference_chars[9] = {'-', '7', 'A', 'b', '9', 'b', 'F', '3', '4'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 9, value, 16);
	tester.check(chars_last == reference_chars + 9 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Hex negative number; implicit base.
void test_m42rz7d24pbcr159()
{
	constexpr std::int32_t reference_value = -2'058'993'460;
	constexpr char reference_chars[11] = {'-', '0', 'x', '7', 'A', 'b', '9', 'b', 'F', '3', '4'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 11, value, 0);
	tester.check(chars_last == reference_chars + 11 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Hex number; implicit base.
void test_d6mz0zwedq5txe0w()
{
	constexpr std::int32_t reference_value = 0x12345678;
	constexpr char reference_chars[10] = {'0', 'x', '1', '2', '3', '4', '5', '6', '7', '8'};
	auto value = std::int32_t{};
	const auto chars_last = bstone::from_chars(reference_chars, reference_chars + 10, value, 0);
	tester.check(chars_last == reference_chars + 10 && value == reference_value);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Base out of range.
void test_blml5orig7eieaqg()
{
	const char chars[1] = {};
	auto value = 0;
	auto is_failed_1 = false;
	auto is_failed_2 = false;

	try
	{
		bstone::from_chars(chars, chars + 1, value, bstone::char_conv_min_base - 1);
	}
	catch (const std::exception&)
	{
		is_failed_1 = true;
	}

	try
	{
		bstone::from_chars(chars, chars + 1, value, bstone::char_conv_max_base + 1);
	}
	catch (const std::exception&)
	{
		is_failed_2 = true;
	}

	tester.check(is_failed_1 && is_failed_2);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Negative number for unsigned type.
void test_frs8x638eej9zgic()
{
	const char chars[2] = {'-', '1'};
	auto value = 0U;
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 2, value);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Unknown base.
void test_x069rmpt1u25xp6w()
{
	const char chars[1] = {'?'};
	auto value = 0;
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 1, value, 0);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// No digits.
void test_bf93ez0o0zk1kl3y()
{
	const char chars[1] = {'+'};
	auto value = 0;
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 1, value, 10);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Invalid digit.
void test_xbaolgk6bxgi3j2d()
{
	const char chars[1] = {'!'};
	auto value = 0;
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 1, value);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Digit out of range.
void test_6kvymcfmze1lvbgb()
{
	const char chars[1] = {'a'};
	auto value = 0;
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 1, value);
	}
	catch (const std::exception&)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// from_chars(const TChar*, const TChar*, TValue&, int)
// Number overflow.
void test_uv2uq0ulf703ivhq()
{
	const char chars[3] = {'2', '0', '0'};
	auto value = std::int8_t{};
	auto is_failed = false;

	try
	{
		bstone::from_chars(chars, chars + 3, value);
	}
	catch (const std::exception&)
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
		register_nibble_to_hex_char();
		register_hex_char_to_nibble();
		register_hex_chars_to_bytes();
		register_bytes_to_hex_chars();
		register_to_chars();
		register_from_chars();
	}

private:
	void register_nibble_to_hex_char()
	{
		tester.register_test("nibble_to_hex_char#786367ma88ppf63d", test_786367ma88ppf63d);
		tester.register_test("nibble_to_hex_char#1kxpwivq3m8uixis", test_1kxpwivq3m8uixis);
	}

	void register_hex_char_to_nibble()
	{
		tester.register_test("hex_char_to_nibble#thc5mvg8pjtzfmvv", test_thc5mvg8pjtzfmvv);
		tester.register_test("hex_char_to_nibble#c47zq4hnypy3m18i", test_c47zq4hnypy3m18i);
	}

	void register_hex_chars_to_bytes()
	{
		tester.register_test("hex_chars_to_bytes#n1b6jxr93yx60dxm", test_n1b6jxr93yx60dxm);
		tester.register_test("hex_chars_to_bytes#kzloe3bvve6i0yzw", test_kzloe3bvve6i0yzw);
		tester.register_test("hex_chars_to_bytes#zm2b65au561avo68", test_zm2b65au561avo68);
		tester.register_test("hex_chars_to_bytes#nxvdm1h98gzatxzo", test_nxvdm1h98gzatxzo);
		tester.register_test("hex_chars_to_bytes#b9cjw5rk6b7jrckg", test_b9cjw5rk6b7jrckg);
		tester.register_test("hex_chars_to_bytes#tfs03rluhhsouvil", test_tfs03rluhhsouvil);
	}

	void register_bytes_to_hex_chars()
	{
		tester.register_test("bytes_to_hex_chars#8qxf14j67dqhycp7", test_8qxf14j67dqhycp7);
		tester.register_test("bytes_to_hex_chars#qej30gonkno07fhs", test_qej30gonkno07fhs);
		tester.register_test("bytes_to_hex_chars#1ghesobjlixkwtjy", test_1ghesobjlixkwtjy);
		tester.register_test("bytes_to_hex_chars#hfno9tjnffo1bciv", test_hfno9tjnffo1bciv);
		tester.register_test("bytes_to_hex_chars#l0rlgeb9nnayge7e", test_l0rlgeb9nnayge7e);
	}

	void register_to_chars()
	{
		tester.register_test("to_chars#6bsk5jbj8h32j51u", test_6bsk5jbj8h32j51u);
		tester.register_test("to_chars#w9v94y8ylfdw0fn6", test_w9v94y8ylfdw0fn6);
		tester.register_test("to_chars#94vh40w9bhfiisuq", test_94vh40w9bhfiisuq);
		tester.register_test("to_chars#dvtkoajkruya2g23", test_dvtkoajkruya2g23);
		tester.register_test("to_chars#gci8hnnnfrzce1ps", test_gci8hnnnfrzce1ps);
		tester.register_test("to_chars#k5dma16zcskflljr", test_k5dma16zcskflljr);
		tester.register_test("to_chars#yu95gjn7r8e549ki", test_yu95gjn7r8e549ki);
		tester.register_test("to_chars#hlb3rbu9n9ru7j0p", test_hlb3rbu9n9ru7j0p);
	}

	void register_from_chars()
	{
		tester.register_test("from_chars#2l8uvpq5jwsq1mey", test_2l8uvpq5jwsq1mey);
		tester.register_test("from_chars#6g2pgwz3xscy7a0s", test_6g2pgwz3xscy7a0s);
		tester.register_test("from_chars#xhrlosdet2rdbpnv", test_xhrlosdet2rdbpnv);
		tester.register_test("from_chars#5aohfvv83ivwlzq0", test_5aohfvv83ivwlzq0);
		tester.register_test("from_chars#ah55jmdmze5tbq7e", test_ah55jmdmze5tbq7e);
		tester.register_test("from_chars#clkb1xo609m44vyu", test_clkb1xo609m44vyu);
		tester.register_test("from_chars#m42rz7d24pbcr159", test_m42rz7d24pbcr159);
		tester.register_test("from_chars#d6mz0zwedq5txe0w", test_d6mz0zwedq5txe0w);
		tester.register_test("from_chars#blml5orig7eieaqg", test_blml5orig7eieaqg);
		tester.register_test("from_chars#frs8x638eej9zgic", test_frs8x638eej9zgic);
		tester.register_test("from_chars#x069rmpt1u25xp6w", test_x069rmpt1u25xp6w);
		tester.register_test("from_chars#bf93ez0o0zk1kl3y", test_bf93ez0o0zk1kl3y);
		tester.register_test("from_chars#xbaolgk6bxgi3j2d", test_xbaolgk6bxgi3j2d);
		tester.register_test("from_chars#6kvymcfmze1lvbgb", test_6kvymcfmze1lvbgb);
		tester.register_test("from_chars#uv2uq0ulf703ivhq", test_uv2uq0ulf703ivhq);
	}
};

auto registrator = Registrator{};

} // namespace
