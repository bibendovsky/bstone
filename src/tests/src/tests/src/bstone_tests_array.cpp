#include <type_traits>

#include "bstone_tester.h"

#include "bstone_array.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// Array()
void test_2x2nmmaqfo9wr278()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{};
	tester.check(a.get_size() == 3 && a.get_data()[0] == 0 && a.get_data()[1] == 0 && a.get_data()[2] == 0);
}

// Array(const Array&)
void test_5apuqpi398r44odk()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a_1 = Type{3, 2, 1};
	constexpr auto a = a_1;
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// Array(const Item (&)[TSize])
void test_4km2w41rqonkecuz()
{
	constexpr int reference_values[3] = {3, 2, 1};
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{reference_values};
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// Array(TArgs&& ...)
void test_amptrvyel4u870vo()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// operator=(const Array& rhs)
void test_kqigzw0pg5f1if3q()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a_1 = Type{3, 2, 1};
	auto a = Type{};
	a = a_1;
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// get_data() const
void test_buevkx23m7v5vsd6()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// get_data()
void test_lv6c5082qvglgz4p()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// get_size()
void test_0jibtmo348m4961h()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};
	tester.check(a.get_size() == 3);
}

// is_empty()
// Empty.
void test_v9231w2h7e3tqxo1()
{
	using Type = bstone::Array<int, 0>;
	constexpr auto a = Type{};
	tester.check(a.is_empty());
}

// is_empty()
// Not empty.
void test_6jqy23kmwi0cq10d()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};
	tester.check(!a.is_empty());
}

// begin() const
void test_tclck0extwmaf4nt()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};

	tester.check(
		std::is_const<std::remove_pointer_t<decltype(a.begin())>>::value &&
		a.begin() == a.get_data());
}

// begin()
void test_k9qcjb1lubacw4jr()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};

	tester.check(
		(!std::is_const<std::remove_pointer_t<decltype(a.begin())>>::value) &&
		a.begin() == a.get_data());
}

// end() const
void test_i6qz7lr1l6i5awsz()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};

	tester.check(
		std::is_const<std::remove_pointer_t<decltype(a.end())>>::value &&
		a.end() == a.get_data() + 3);
}

// end()
void test_meck0cvc6k5n9ijv()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};

	tester.check(
		(!std::is_const<std::remove_pointer_t<decltype(a.end())>>::value) &&
		a.end() == a.get_data() + 3);
}

// cbegin()
void test_cmpqg9q9vdudjx8q()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};

	tester.check(
		std::is_const<std::remove_pointer_t<decltype(a.cbegin())>>::value &&
		a.cbegin() == a.get_data());
}

// cend() const
void test_gt2wbhwg50k1jt61()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};

	tester.check(
		std::is_const<std::remove_pointer_t<decltype(a.cend())>>::value &&
		a.cend() == a.get_data() + 3);
}

// fill(Item)
void test_ced8u7attfzfhvs8()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{0, 0, 0};
	a.fill(3);
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 3 && a.get_data()[2] == 3);
}

// operator[](ArrayInt) const
void test_jwkyyqpg238ud80o()
{
	using Type = bstone::Array<int, 3>;
	constexpr auto a = Type{3, 2, 1};

	tester.check(
		std::is_const<std::remove_reference_t<decltype(a[0])>>::value &&
		a.get_size() == 3 && a[1] == 2);
}

// operator[](ArrayInt) const
void test_s6yxwlof7c5m33ln()
{
	using Type = bstone::Array<int, 3>;
	auto a = Type{3, 2, 1};

	tester.check(
		!std::is_const<std::remove_reference_t<decltype(a[0])>>::value &&
		a.get_size() == 3 && a[1] == 2);
}

// swap(Array&)
void test_xnjkdwes4gq6lzer()
{
	using Type = bstone::Array<int, 3>;
	auto a_1 = Type{3, 2, 1};
	auto a = Type{};
	a.swap(a_1);
	tester.check(a.get_size() == 3 && a.get_data()[0] == 3 && a.get_data()[1] == 2 && a.get_data()[2] == 1);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_array();
	}

private:
	void register_array()
	{
		tester.register_test("Array#2x2nmmaqfo9wr278", test_2x2nmmaqfo9wr278);
		tester.register_test("Array#5apuqpi398r44odk", test_5apuqpi398r44odk);
		tester.register_test("Array#4km2w41rqonkecuz", test_4km2w41rqonkecuz);
		tester.register_test("Array#amptrvyel4u870vo", test_amptrvyel4u870vo);
		tester.register_test("Array#kqigzw0pg5f1if3q", test_kqigzw0pg5f1if3q);
		tester.register_test("Array#buevkx23m7v5vsd6", test_buevkx23m7v5vsd6);
		tester.register_test("Array#lv6c5082qvglgz4p", test_lv6c5082qvglgz4p);
		tester.register_test("Array#0jibtmo348m4961h", test_0jibtmo348m4961h);
		tester.register_test("Array#v9231w2h7e3tqxo1", test_v9231w2h7e3tqxo1);
		tester.register_test("Array#6jqy23kmwi0cq10d", test_6jqy23kmwi0cq10d);
		tester.register_test("Array#tclck0extwmaf4nt", test_tclck0extwmaf4nt);
		tester.register_test("Array#k9qcjb1lubacw4jr", test_k9qcjb1lubacw4jr);
		tester.register_test("Array#i6qz7lr1l6i5awsz", test_i6qz7lr1l6i5awsz);
		tester.register_test("Array#meck0cvc6k5n9ijv", test_meck0cvc6k5n9ijv);
		tester.register_test("Array#cmpqg9q9vdudjx8q", test_cmpqg9q9vdudjx8q);
		tester.register_test("Array#gt2wbhwg50k1jt61", test_gt2wbhwg50k1jt61);
		tester.register_test("Array#ced8u7attfzfhvs8", test_ced8u7attfzfhvs8);
		tester.register_test("Array#jwkyyqpg238ud80o", test_jwkyyqpg238ud80o);
		tester.register_test("Array#s6yxwlof7c5m33ln", test_s6yxwlof7c5m33ln);
		tester.register_test("Array#xnjkdwes4gq6lzer", test_xnjkdwes4gq6lzer);
	}
};

auto registrator = Registrator{};

} // namespace
