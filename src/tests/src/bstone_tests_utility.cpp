#include "bstone_tester.h"

#include "bstone_utility.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// swap(A, B)
void test_dirxqh5kufqeux2t()
{
	constexpr auto reference_1 = 12345;
	constexpr auto reference_2 = 67890;
	auto a = reference_1;
	auto b = reference_2;
	bstone::Utility::swap(a, b);
	tester.check(a == reference_2 && b == reference_1);
}

// swap_ranges(Ab, Ae, Bb)
void test_opzs9oj70deyvo2i()
{
	int a[4] = {1, 2, 3, 4};
	int b[4] = {5, 6, 7, 8};

	bstone::Utility::swap_ranges(a, a + 4, b);

	tester.check(
		a[0] == 5 && b[0] == 1 &&
		a[1] == 6 && b[1] == 2 &&
		a[2] == 7 && b[2] == 3 &&
		a[3] == 8 && b[3] == 4);
}

// swap_ranges(Ab, Ae, Bb)
void test_cxf6khrb1efyd20l()
{
	int a[4] = {1, 2, 3, 4};
	int b[4] = {5, 6, 7, 8};

	bstone::Utility::swap(a, b);

	tester.check(
		a[0] == 5 && b[0] == 1 &&
		a[1] == 6 && b[1] == 2 &&
		a[2] == 7 && b[2] == 3 &&
		a[3] == 8 && b[3] == 4);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_swap();
	}

private:
	void register_swap()
	{
		tester.register_test("Utility::swap#dirxqh5kufqeux2t", test_dirxqh5kufqeux2t);
		tester.register_test("Utility::swap_ranges#opzs9oj70deyvo2i", test_opzs9oj70deyvo2i);
		tester.register_test("Utility::swap#cxf6khrb1efyd20l", test_cxf6khrb1efyd20l);
	}
};

auto registrator = Registrator{};

} // namespace
