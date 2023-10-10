#include "bstone_tester.h"

#include "bstone_char_traits.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

void test_03xkg906j47u77nu()
{
	constexpr auto size = bstone::char_traits::get_size("test");
	tester.check(size == 4);
}

// ==========================================================================

// Same length, left is less then the right.
void test_47dc1f3yc86pquh3()
{
	constexpr auto result = bstone::char_traits::compare("test", 4, "uest", 4);
	tester.check(result < 0);
}

// Same length, left is greater then the right.
void test_pxdmk4bok84w30fx()
{
	constexpr auto result = bstone::char_traits::compare("uest", 4, "test", 4);
	tester.check(result > 0);
}

// Same length, equal sides.
void test_t60749c01tqedcdf()
{
	constexpr auto result = bstone::char_traits::compare("test", 4, "test", 4);
	tester.check(result == 0);
}

// Left shorter, same characters.
void test_s93zdy16bnmbo6pd()
{
	constexpr auto result = bstone::char_traits::compare("test", 4, "test_", 5);
	tester.check(result < 0);
}

// Right shorter, same characters.
void test_e8fl6nxygp91hr1k()
{
	constexpr auto result = bstone::char_traits::compare("test_", 5, "test", 4);
	tester.check(result > 0);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_get_size();
		register_compare();
	}

private:
	void register_get_size()
	{
		tester.register_test("char_traits::get_size#03xkg906j47u77nu", test_03xkg906j47u77nu);
	}

	void register_compare()
	{
		tester.register_test("char_traits::compare#47dc1f3yc86pquh3", test_47dc1f3yc86pquh3);
		tester.register_test("char_traits::compare#pxdmk4bok84w30fx", test_pxdmk4bok84w30fx);
		tester.register_test("char_traits::compare#t60749c01tqedcdf", test_t60749c01tqedcdf);
		tester.register_test("char_traits::compare#s93zdy16bnmbo6pd", test_s93zdy16bnmbo6pd);
		tester.register_test("char_traits::compare#e8fl6nxygp91hr1k", test_e8fl6nxygp91hr1k);
	}
};

auto registrator = Registrator{};

} // namespace
