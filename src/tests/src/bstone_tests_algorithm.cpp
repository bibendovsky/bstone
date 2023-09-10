#include "bstone_algorithm.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// clamp(const T&, const T&, const T&, TPredicate)
// Value inside a range.
void test_oueqvtfkn3z4lrj8()
{
	constexpr auto x = 3;
	constexpr auto x_min = 2;
	constexpr auto x_max = 4;
	const auto clamped_x = bstone::clamp(x, x_min, x_max);
	tester.check(clamped_x == x);
}

// clamp(const T&, const T&, const T&, TPredicate)
// Value less than minimum.
void test_l4m6chrhta6vy92o()
{
	constexpr auto x = 1;
	constexpr auto x_min = 2;
	constexpr auto x_max = 4;
	const auto clamped_x = bstone::clamp(x, x_min, x_max);
	tester.check(clamped_x == x_min);
}

// clamp(const T&, const T&, const T&, TPredicate)
// Value greater than maximum.
void test_y3aacbj4aehhs11k()
{
	constexpr auto x = 5;
	constexpr auto x_min = 2;
	constexpr auto x_max = 4;
	const auto clamped_x = bstone::clamp(x, x_min, x_max);
	tester.check(clamped_x == x_max);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_clamp();
	}

private:
	void register_clamp()
	{
		tester.register_test("clamp#oueqvtfkn3z4lrj8", test_oueqvtfkn3z4lrj8);
		tester.register_test("clamp#l4m6chrhta6vy92o", test_l4m6chrhta6vy92o);
		tester.register_test("clamp#y3aacbj4aehhs11k", test_y3aacbj4aehhs11k);
	}
};

auto registrator = Registrator{};

} // namespace
