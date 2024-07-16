#include "bstone_tester.h"

#include "bstone_sha1.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// Sha1()
void test_q7lcg7h1ggusgs5p()
{
	const auto ref_sha1_digest = bstone::Sha1Digest{};
	const auto sha1 = bstone::Sha1{};
	const auto& sha1_digest = sha1.get_digest();
	tester.check(sha1_digest == ref_sha1_digest);
}

// Sha1(const Sha1&)
void test_yf9kdxfve6tepub1()
{
	auto a = bstone::Sha1{};
	const std::uint8_t m[4] = {1, 2, 3, 4};
	a.process(m, 4);
	a.finish();
	const auto a_digest = a.get_digest();
	const auto b = a;
	const auto b_digest = b.get_digest();
	tester.check(a_digest == b_digest);
}

// Sha1& operator=(const Sha1&)
void test_nsoiz0vhriaphr1m()
{
	auto a = bstone::Sha1{};
	const std::uint8_t m[4] = {1, 2, 3, 4};
	a.process(m, 4);
	a.finish();
	const auto a_digest = a.get_digest();
	auto b = bstone::Sha1{};
	b = a;
	const auto b_digest = b.get_digest();
	tester.check(a_digest == b_digest);
}

// process(const std::uint8_t*, std::intptr_t) and finish()
void test_lpjru7shzc0ctgsw()
{
	constexpr std::uint8_t ref_bytes[] = {0x54, 0x65, 0x73, 0x74}; // "Test"
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		static_cast<std::uint8_t>(0x64),
		static_cast<std::uint8_t>(0x0A),
		static_cast<std::uint8_t>(0xB2),
		static_cast<std::uint8_t>(0xBA),
		static_cast<std::uint8_t>(0xE0),
		static_cast<std::uint8_t>(0x7B),
		static_cast<std::uint8_t>(0xED),
		static_cast<std::uint8_t>(0xC4),
		static_cast<std::uint8_t>(0xC1),
		static_cast<std::uint8_t>(0x63),
		static_cast<std::uint8_t>(0xF6),
		static_cast<std::uint8_t>(0x79),
		static_cast<std::uint8_t>(0xA7),
		static_cast<std::uint8_t>(0x46),
		static_cast<std::uint8_t>(0xF7),
		static_cast<std::uint8_t>(0xAB),
		static_cast<std::uint8_t>(0x7F),
		static_cast<std::uint8_t>(0xB5),
		static_cast<std::uint8_t>(0xD1),
		static_cast<std::uint8_t>(0xFA)
	};

	auto sha1 = bstone::Sha1{};
	sha1.process(ref_bytes, sizeof(ref_bytes));
	sha1.finish();
	const auto digest = sha1.get_digest();
	tester.check(digest == ref_digest);
}

// process(Span<const std::uint8_t>) and finish()
void test_pmq9ub20p7haxiyv()
{
	constexpr std::uint8_t ref_bytes[] = {0x54, 0x65, 0x73, 0x74}; // "Test"
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		static_cast<std::uint8_t>(0x64),
		static_cast<std::uint8_t>(0x0A),
		static_cast<std::uint8_t>(0xB2),
		static_cast<std::uint8_t>(0xBA),
		static_cast<std::uint8_t>(0xE0),
		static_cast<std::uint8_t>(0x7B),
		static_cast<std::uint8_t>(0xED),
		static_cast<std::uint8_t>(0xC4),
		static_cast<std::uint8_t>(0xC1),
		static_cast<std::uint8_t>(0x63),
		static_cast<std::uint8_t>(0xF6),
		static_cast<std::uint8_t>(0x79),
		static_cast<std::uint8_t>(0xA7),
		static_cast<std::uint8_t>(0x46),
		static_cast<std::uint8_t>(0xF7),
		static_cast<std::uint8_t>(0xAB),
		static_cast<std::uint8_t>(0x7F),
		static_cast<std::uint8_t>(0xB5),
		static_cast<std::uint8_t>(0xD1),
		static_cast<std::uint8_t>(0xFA)
	};

	auto sha1 = bstone::Sha1{};
	sha1.process(bstone::make_const_span(ref_bytes));
	sha1.finish();
	const auto digest = sha1.get_digest();
	tester.check(digest == ref_digest);
}

// process(Span<std::uint8_t>) and finish()
void test_hka86bzux5vmorkw()
{
	std::uint8_t ref_bytes[] = {0x54, 0x65, 0x73, 0x74}; // "Test"

	constexpr auto ref_digest = bstone::Sha1Digest
	{
		static_cast<std::uint8_t>(0x64),
		static_cast<std::uint8_t>(0x0A),
		static_cast<std::uint8_t>(0xB2),
		static_cast<std::uint8_t>(0xBA),
		static_cast<std::uint8_t>(0xE0),
		static_cast<std::uint8_t>(0x7B),
		static_cast<std::uint8_t>(0xED),
		static_cast<std::uint8_t>(0xC4),
		static_cast<std::uint8_t>(0xC1),
		static_cast<std::uint8_t>(0x63),
		static_cast<std::uint8_t>(0xF6),
		static_cast<std::uint8_t>(0x79),
		static_cast<std::uint8_t>(0xA7),
		static_cast<std::uint8_t>(0x46),
		static_cast<std::uint8_t>(0xF7),
		static_cast<std::uint8_t>(0xAB),
		static_cast<std::uint8_t>(0x7F),
		static_cast<std::uint8_t>(0xB5),
		static_cast<std::uint8_t>(0xD1),
		static_cast<std::uint8_t>(0xFA)
	};

	auto sha1 = bstone::Sha1{};
	sha1.process(bstone::make_const_span(ref_bytes));
	sha1.finish();
	const auto digest = sha1.get_digest();
	tester.check(digest == ref_digest);
}

// ==========================================================================

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// char
void test_0km1jccde7pdny02()
{
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		std::uint8_t{0xB6}, std::uint8_t{0x58}, std::uint8_t{0x9F}, std::uint8_t{0xC6}, std::uint8_t{0xAB},
		std::uint8_t{0x0D}, std::uint8_t{0xC8}, std::uint8_t{0x2C}, std::uint8_t{0xF1}, std::uint8_t{0x20},
		std::uint8_t{0x99}, std::uint8_t{0xD1}, std::uint8_t{0xC2}, std::uint8_t{0xD4}, std::uint8_t{0x0A},
		std::uint8_t{0xB9}, std::uint8_t{0x94}, std::uint8_t{0xE8}, std::uint8_t{0x41}, std::uint8_t{0x0C},
	};

	constexpr auto digest = bstone::make_sha1_digest(
		"b6589fc6ab0dc82cf12099d1c2d40aB994E8410C???", bstone::sha1_digest_char_count);

	constexpr auto is_valid = digest == ref_digest;

	tester.check(is_valid);
}

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// char
// Invalid string length.
void test_5mzjwm9llcksb1og()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest("b6589fc6ab0dc82cf12099d1c2d40aB994E8410C", 3);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// char
// Invalid string.
void test_osnn3bobbkydx33t()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest("????????????????????????????????????????", bstone::sha1_digest_char_count);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// wchar_t
void test_0i4ic6gnbw9z4tij()
{
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		std::uint8_t{0xB6}, std::uint8_t{0x58}, std::uint8_t{0x9F}, std::uint8_t{0xC6}, std::uint8_t{0xAB},
		std::uint8_t{0x0D}, std::uint8_t{0xC8}, std::uint8_t{0x2C}, std::uint8_t{0xF1}, std::uint8_t{0x20},
		std::uint8_t{0x99}, std::uint8_t{0xD1}, std::uint8_t{0xC2}, std::uint8_t{0xD4}, std::uint8_t{0x0A},
		std::uint8_t{0xB9}, std::uint8_t{0x94}, std::uint8_t{0xE8}, std::uint8_t{0x41}, std::uint8_t{0x0C},
	};

	constexpr auto digest = bstone::make_sha1_digest(
		L"b6589fc6ab0dc82cf12099d1c2d40aB994E8410C???", bstone::sha1_digest_char_count);

	constexpr auto is_valid = digest == ref_digest;

	tester.check(is_valid);
}

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// char
// Invalid string length.
void test_p4yyydr8g9eoyunn()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest(L"b6589fc6ab0dc82cf12099d1c2d40aB994E8410C", 3);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// constexpr Sha1Digest make_sha1_digest(const TChar*, std::intptr_t)
// char
// Invalid string.
void test_z5zvdoi7mnfotb62()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest(L"????????????????????????????????????????", bstone::sha1_digest_char_count);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// Sha1Digest make_sha1_digest(const TChar*)
// char
void test_ifkd3ng0v38k7fhx()
{
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		std::uint8_t{0xB6}, std::uint8_t{0x58}, std::uint8_t{0x9F}, std::uint8_t{0xC6}, std::uint8_t{0xAB},
		std::uint8_t{0x0D}, std::uint8_t{0xC8}, std::uint8_t{0x2C}, std::uint8_t{0xF1}, std::uint8_t{0x20},
		std::uint8_t{0x99}, std::uint8_t{0xD1}, std::uint8_t{0xC2}, std::uint8_t{0xD4}, std::uint8_t{0x0A},
		std::uint8_t{0xB9}, std::uint8_t{0x94}, std::uint8_t{0xE8}, std::uint8_t{0x41}, std::uint8_t{0x0C},
	};

	constexpr auto digest = bstone::make_sha1_digest("b6589fc6ab0dc82cf12099d1c2d40aB994E8410C");
	constexpr auto is_valid = digest == ref_digest;

	tester.check(is_valid);
}

// Sha1Digest make_sha1_digest(const TChar*)
// char
// Invalid string.
void test_oxsseq12zq4gc0mh()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest("????????????????????????????????????????");
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// Sha1Digest make_sha1_digest(const TChar*)
// wchar_t
void test_de36ayiiisb95y67()
{
	constexpr auto ref_digest = bstone::Sha1Digest
	{
		std::uint8_t{0xB6}, std::uint8_t{0x58}, std::uint8_t{0x9F}, std::uint8_t{0xC6}, std::uint8_t{0xAB},
		std::uint8_t{0x0D}, std::uint8_t{0xC8}, std::uint8_t{0x2C}, std::uint8_t{0xF1}, std::uint8_t{0x20},
		std::uint8_t{0x99}, std::uint8_t{0xD1}, std::uint8_t{0xC2}, std::uint8_t{0xD4}, std::uint8_t{0x0A},
		std::uint8_t{0xB9}, std::uint8_t{0x94}, std::uint8_t{0xE8}, std::uint8_t{0x41}, std::uint8_t{0x0C},
	};

	constexpr auto digest = bstone::make_sha1_digest(L"b6589fc6ab0dc82cf12099d1c2d40aB994E8410C");
	constexpr auto is_valid = digest == ref_digest;
	tester.check(is_valid);
}

// Sha1Digest make_sha1_digest(const TChar*)
// char
// Invalid string.
void test_fzf597nw6linsoxi()
{
	auto is_failed = false;

	try
	{
		bstone::make_sha1_digest(L"????????????????????????????????????????", bstone::sha1_digest_char_count);
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
		register_sha1();
		register_make_sha1_digest();
	}

private:
	void register_sha1()
	{
		tester.register_test("Sha1#q7lcg7h1ggusgs5p", test_q7lcg7h1ggusgs5p);
		tester.register_test("Sha1#yf9kdxfve6tepub1", test_yf9kdxfve6tepub1);
		tester.register_test("Sha1#nsoiz0vhriaphr1m", test_nsoiz0vhriaphr1m);
		tester.register_test("Sha1#lpjru7shzc0ctgsw", test_lpjru7shzc0ctgsw);
		tester.register_test("Sha1#pmq9ub20p7haxiyv", test_pmq9ub20p7haxiyv);
		tester.register_test("Sha1#hka86bzux5vmorkw", test_hka86bzux5vmorkw);
	}

	void register_make_sha1_digest()
	{
		tester.register_test("make_sha1_digest#0km1jccde7pdny02", test_0km1jccde7pdny02);
		tester.register_test("make_sha1_digest#5mzjwm9llcksb1og", test_5mzjwm9llcksb1og);
		tester.register_test("make_sha1_digest#osnn3bobbkydx33t", test_osnn3bobbkydx33t);

		tester.register_test("make_sha1_digest#0i4ic6gnbw9z4tij", test_0i4ic6gnbw9z4tij);
		tester.register_test("make_sha1_digest#p4yyydr8g9eoyunn", test_p4yyydr8g9eoyunn);
		tester.register_test("make_sha1_digest#z5zvdoi7mnfotb62", test_z5zvdoi7mnfotb62);

		tester.register_test("make_sha1_digest#ifkd3ng0v38k7fhx", test_ifkd3ng0v38k7fhx);
		tester.register_test("make_sha1_digest#oxsseq12zq4gc0mh", test_oxsseq12zq4gc0mh);

		tester.register_test("make_sha1_digest#de36ayiiisb95y67", test_de36ayiiisb95y67);
		tester.register_test("make_sha1_digest#fzf597nw6linsoxi", test_fzf597nw6linsoxi);
	}
};

auto registrator = Registrator{};

} // namespace
