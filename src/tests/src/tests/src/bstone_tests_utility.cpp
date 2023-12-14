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
	bstone::swop(a, b);
	tester.check(a == reference_2 && b == reference_1);
}

// swap_ranges(Ab, Ae, Bb)
void test_opzs9oj70deyvo2i()
{
	int a[4] = {1, 2, 3, 4};
	int b[4] = {5, 6, 7, 8};

	bstone::swop_ranges(a, a + 4, b);

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

	bstone::swop(a, b);

	tester.check(
		a[0] == 5 && b[0] == 1 &&
		a[1] == 6 && b[1] == 2 &&
		a[2] == 7 && b[2] == 3 &&
		a[3] == 8 && b[3] == 4);
}

// ==========================================================================

void test_zb0b1ipzmya72nqc()
{
	auto x = 1;
	const auto xc = bstone::as_const(x);
	tester.check(xc == x);
}

// ==========================================================================

// T& as_mutable(const T&) noexcept
void test_i2temy08nte1vcy7()
{
	const auto x = 1;
	auto& xc = bstone::as_mutable(x);
	xc = 2;
	tester.check(*(static_cast<const int*>(static_cast<const void*>(&x))) == 2);
}

// T* as_mutable(const T*) noexcept
void test_28urwoy5fqjbc8r5()
{
	const auto x = 1;
	auto xc = bstone::as_mutable(&x);
	*xc = 2;
	tester.check(*(static_cast<const int*>(static_cast<const void*>(&x))) == 2);
}

// T* as_mutable(T*) noexcept
void test_02ojqcmo8al8ivai()
{
	auto x = 1;
	auto xc = bstone::as_mutable(&x);
	*xc = 2;
	tester.check(*(static_cast<const int*>(static_cast<const void*>(&x))) == 2);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_swap();
		register_as_const();
		register_as_mutable();
	}

private:
	void register_swap()
	{
		tester.register_test("swop#dirxqh5kufqeux2t", test_dirxqh5kufqeux2t);
		tester.register_test("swop_ranges#opzs9oj70deyvo2i", test_opzs9oj70deyvo2i);
		tester.register_test("swop#zb0b1ipzmya72nqc", test_zb0b1ipzmya72nqc);
	}

	void register_as_const()
	{
		tester.register_test("as_const#cxf6khrb1efyd20l", test_cxf6khrb1efyd20l);
	}

	void register_as_mutable()
	{
		tester.register_test("as_mutable#i2temy08nte1vcy7", test_i2temy08nte1vcy7);
		tester.register_test("as_mutable#28urwoy5fqjbc8r5", test_28urwoy5fqjbc8r5);
		tester.register_test("as_mutable#02ojqcmo8al8ivai", test_02ojqcmo8al8ivai);
	}
};

auto registrator = Registrator{};

} // namespace
