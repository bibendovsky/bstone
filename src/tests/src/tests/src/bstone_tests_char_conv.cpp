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

class Registrator
{
public:
	Registrator()
	{
		register_nibble_to_hex_char();
		register_hex_char_to_nibble();
		register_hex_chars_to_bytes();
		register_bytes_to_hex_chars();
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
};

auto registrator = Registrator{};

} // namespace
