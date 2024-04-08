#include "bstone_four_cc.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// FourCc() noexcept
void test_e8spipwesrxs9pkr()
{
	constexpr auto four_cc = bstone::FourCc{};
	const auto is_valid_value = four_cc.get_value() == 0;
	tester.check(is_valid_value);
}

// FourCc(std::uint32_t) noexcept
void test_l5lraalljuqar2ks()
{
	constexpr auto four_cc = bstone::FourCc{0x01020304U};
	const auto is_valid_value = four_cc.get_value() == 0x01020304U;
	tester.check(is_valid_value);
}

// FourCc(TChar, TChar, TChar, TChar) noexcept
void test_ojgctttjq03zk803()
{
	constexpr auto four_cc = bstone::FourCc{'0', '1', '2', '3'};
	const auto is_valid_value = four_cc.get_value() == 0x33323130U;
	tester.check(is_valid_value);
}

// FourCc(const FourCc&) noexcept
void test_oz3ffjua2m5aoh3x()
{
	constexpr auto four_cc_1 = bstone::FourCc{0x01020304U};
	constexpr auto four_cc = four_cc_1;
	const auto is_valid_value = four_cc.get_value() == 0x01020304U;
	tester.check(is_valid_value);
}

// ==========================================================================

// FourCc& operator=(const FourCc&) noexcept
void test_nsq0yax7xgxmat6i()
{
	constexpr auto four_cc_1 = bstone::FourCc{0x01020304U};
	auto four_cc = bstone::FourCc{};
	four_cc = four_cc_1;
	const auto is_valid_value = four_cc.get_value() == 0x01020304U;
	tester.check(is_valid_value);
}

// ==========================================================================

// std::uint32_t get_value() const noexcept
void test_o0w0veqqj3j3ug8s()
{
	constexpr auto four_cc = bstone::FourCc{0x01020304U};
	const auto is_valid_value = four_cc.get_value() == 0x01020304U;
	tester.check(is_valid_value);
}

// ==========================================================================

// char operator[](std::intptr_t) const noexcept
void test_cypcn5gej0ddr0u6()
{
	constexpr auto four_cc = bstone::FourCc{'0', '1', '2', '3'};

	constexpr auto is_valid_value =
		four_cc[0] == '0' &&
		four_cc[1] == '1' &&
		four_cc[2] == '2' &&
		four_cc[3] == '3';

	tester.check(is_valid_value);
}

// ==========================================================================

// bool operator==(const FourCc&, const FourCc&) noexcept
void test_c9g9s7mj4yip5p5k()
{
	constexpr auto four_cc_1 = bstone::FourCc{'0', '1', '2', '3'};
	constexpr auto four_cc_2 = bstone::FourCc{'0', '1', '2', '3'};
	constexpr auto is_valid_value = four_cc_1 == four_cc_2;
	tester.check(is_valid_value);
}

// ==========================================================================

// bool operator!=(const FourCc&, const FourCc&) noexcept
void test_rcn3ldo9kmvlbicv()
{
	constexpr auto four_cc_1 = bstone::FourCc{'_', '1', '2', '3'};
	constexpr auto four_cc_2 = bstone::FourCc{'0', '1', '2', '3'};
	constexpr auto is_valid_value = four_cc_1 != four_cc_2;
	tester.check(is_valid_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_assignment_operator();
		register_get_value();
		register_subscript_operator();
		register_equality_operator();
		register_inequality_operator();
	}

private:
	void register_ctor()
	{
		tester.register_test("FourCc#e8spipwesrxs9pkr", test_e8spipwesrxs9pkr);
		tester.register_test("FourCc#l5lraalljuqar2ks", test_l5lraalljuqar2ks);
		tester.register_test("FourCc#ojgctttjq03zk803", test_ojgctttjq03zk803);
		tester.register_test("FourCc#oz3ffjua2m5aoh3x", test_oz3ffjua2m5aoh3x);
	}

	void register_assignment_operator()
	{
		tester.register_test("FourCc#nsq0yax7xgxmat6i", test_nsq0yax7xgxmat6i);
	}

	void register_get_value()
	{
		tester.register_test("FourCc#o0w0veqqj3j3ug8s", test_o0w0veqqj3j3ug8s);
	}

	void register_subscript_operator()
	{
		tester.register_test("FourCc#cypcn5gej0ddr0u6", test_cypcn5gej0ddr0u6);
	}

	void register_equality_operator()
	{
		tester.register_test("FourCc#c9g9s7mj4yip5p5k", test_c9g9s7mj4yip5p5k);
	}

	void register_inequality_operator()
	{
		tester.register_test("FourCc#rcn3ldo9kmvlbicv", test_rcn3ldo9kmvlbicv);
	}
};

auto registrator = Registrator{};

} // namespace
