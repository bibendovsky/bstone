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

class Registrator
{
public:
	Registrator()
	{
		register_sha1();
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
};

auto registrator = Registrator{};

} // namespace
