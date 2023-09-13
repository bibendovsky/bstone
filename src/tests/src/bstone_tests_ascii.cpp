#include <algorithm>
#include <iterator>

#include "bstone_tester.h"

#include "bstone_ascii.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// is_binary(TChar)
void test_3n9te47slod95ayp()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_binary(c);

		switch (c)
		{
			case '0':
			case '1':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// is_octal(TChar)
void test_2xp3lt9qtg0fu1ao()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_octal(c);

		switch (c)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// is_decimal(TChar)
void test_bwkigyyla764iog3()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_decimal(c);

		switch (c)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// is_hex(TChar)
void test_xj2qah2eyxrpiliu()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_hex(c);

		switch (c)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// is_lower(TChar)
void test_pgj83zrmep9j1wxm()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_lower(c);

		switch (c)
		{
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// is_upper(TChar)
void test_e0ilqdc6d0scb5hg()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto is = bstone::ascii::is_upper(c);

		switch (c)
		{
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
				result &= is == true;
				break;

			default:
				result &= is == false;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// to_lower(TChar)
void test_fssuexqd5oyt6y8u()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto lc = bstone::ascii::to_lower(c);

		switch (c)
		{
			case 'A':
			case 'B':
			case 'C':
			case 'D':
			case 'E':
			case 'F':
			case 'G':
			case 'H':
			case 'I':
			case 'J':
			case 'K':
			case 'L':
			case 'M':
			case 'N':
			case 'O':
			case 'P':
			case 'Q':
			case 'R':
			case 'S':
			case 'T':
			case 'U':
			case 'V':
			case 'W':
			case 'X':
			case 'Y':
			case 'Z':
				result &= c + 0x20 == lc;
				break;

			default:
				result &= c == lc;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// to_lower(TIter, TIter)
void test_8vh3r8i5ua9osxjn()
{
	constexpr auto char_count = 4;
	const char reference_chars[char_count] = {'a', 'b', 'c', '?'};
	char chars[char_count] = {'A', 'b', 'C', '?'};
	bstone::ascii::to_lower(std::begin(chars), std::end(chars));
	tester.check(std::equal(chars, chars + char_count, reference_chars));
}

// ==========================================================================

// to_upper(TChar)
void test_hv40djx2iy3bagwi()
{
	auto result = true;

	for (auto i = 0; i < 256; ++i)
	{
		const auto c = static_cast<char>(i);
		const auto uc = bstone::ascii::to_upper(c);

		switch (c)
		{
			case 'a':
			case 'b':
			case 'c':
			case 'd':
			case 'e':
			case 'f':
			case 'g':
			case 'h':
			case 'i':
			case 'j':
			case 'k':
			case 'l':
			case 'm':
			case 'n':
			case 'o':
			case 'p':
			case 'q':
			case 'r':
			case 's':
			case 't':
			case 'u':
			case 'v':
			case 'w':
			case 'x':
			case 'y':
			case 'z':
				result &= c == uc + 0x20;
				break;

			default:
				result &= c == uc;
				break;
		}
	}

	tester.check(result);
}

// ==========================================================================

// to_upper(TIter, TIter)
void test_go2oxu950avooimt()
{
	constexpr auto char_count = 4;
	const char reference_chars[char_count] = {'A', 'B', 'C', '?'};
	char chars[char_count] = {'a', 'b', 'C', '?'};
	bstone::ascii::to_upper(std::begin(chars), std::end(chars));
	tester.check(std::equal(chars, chars + char_count, reference_chars));
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_is_binary();
		register_is_octal();
		register_is_decimal();
		register_is_hex();
		register_is_lower();
		register_is_upper();
		register_to_lower();
		register_to_upper();
	}

private:
	void register_is_binary()
	{
		tester.register_test("ascii::is_binary#3n9te47slod95ayp", test_3n9te47slod95ayp);
	}

	void register_is_octal()
	{
		tester.register_test("ascii::is_octal#2xp3lt9qtg0fu1ao", test_2xp3lt9qtg0fu1ao);
	}

	void register_is_decimal()
	{
		tester.register_test("ascii::is_decimal#bwkigyyla764iog3", test_bwkigyyla764iog3);
	}

	void register_is_hex()
	{
		tester.register_test("ascii::is_hex#xj2qah2eyxrpiliu", test_xj2qah2eyxrpiliu);
	}

	void register_is_lower()
	{
		tester.register_test("ascii::is_lower#pgj83zrmep9j1wxm", test_pgj83zrmep9j1wxm);
	}

	void register_is_upper()
	{
		tester.register_test("ascii::is_upper#e0ilqdc6d0scb5hg", test_e0ilqdc6d0scb5hg);
	}

	void register_to_lower()
	{
		tester.register_test("ascii::to_lower#fssuexqd5oyt6y8u", test_fssuexqd5oyt6y8u);
		tester.register_test("ascii::to_lower#8vh3r8i5ua9osxjn", test_8vh3r8i5ua9osxjn);
	}

	void register_to_upper()
	{
		tester.register_test("ascii::to_upper#hv40djx2iy3bagwi", test_hv40djx2iy3bagwi);
		tester.register_test("ascii::to_upper#go2oxu950avooimt", test_go2oxu950avooimt);
	}
};

auto registrator = Registrator{};

} // namespace
