#include <algorithm>
#include <type_traits>

#include "bstone_tester.h"

#include "bstone_span.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// Span()
void test_ue5mzqprtx1lm0q7()
{
	constexpr auto span = bstone::Span<const int>{};
	tester.check(span.get_data() == nullptr && span.get_size() == 0);
}

// Span(Item*, SpanInt)
void test_azyjatmoxj6k1qnh()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values, value_count};

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// Span(Item (&)[TSize])
void test_urx4i2uyw7z1k1di()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// Span(const Span&)
void test_yxxe9qkpjtkmkwv7()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span_1 = bstone::Span<const int>{values};
	constexpr auto span = span_1;

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// operator=(const Span&)
void test_6u8ua35z28xhz00j()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span_1 = bstone::Span<const int>{values};
	auto span = bstone::Span<const int>{};
	span = span_1;

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// get_data()
void test_9qwtugkqkrq0snp9()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.get_data() == values);
}

// get_size()
void test_uhhdp4y73ufes1he()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.get_size() == value_count);
}

// has_data()
// Not empty.
void test_js5k7pyff5xm50gs()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.has_data());
}

// has_data()
// Empty.
void test_du81z5yyfuv6vy3b()
{
	constexpr auto span = bstone::Span<const int>{};
	tester.check(!span.has_data());
}

// is_empty()
// Not empty.
void test_y1brl21aurhjqpb3()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(!span.is_empty());
}

// is_empty()
// Empty.
void test_9g02a0e5r2iba386()
{
	constexpr auto span = bstone::Span<const int>{};
	tester.check(span.is_empty());
}

// get_bytes_size()
void test_y82j43v4a54wo211()
{
	using Type = int;
	constexpr auto value_count = 3;
	constexpr auto bytes_count = value_count * static_cast<bstone::SpanInt>(sizeof(Type));
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const Type>{values};
	tester.check(span.get_bytes_size() == bytes_count);
}

// begin()
void test_k93ls3eot86tjqyl()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.begin() == values);
}

// end()
void test_p9atqnjpm3ecfp2u()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.end() == values + value_count);
}

// get_front() const
void test_rrn85kwix9aioxj5()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.get_front() == values[0]);
}

// get_front()
void test_dgbwer4gkwz73ywp()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	auto span = bstone::Span<const int>{values};
	tester.check(span.get_front() == values[0]);
}

// get_back() const
void test_g1577nxmqwje2789()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.get_back() == values[value_count - 1]);
}

// get_back()
void test_sq9gg9qdh2jeomnr()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	auto span = bstone::Span<const int>{values};
	tester.check(span.get_back() == values[value_count - 1]);
}

// get_subspan(SpanInt, SpanInt) const
void test_pjsw5n3iw8y9l8gm()
{
	constexpr auto value_count = 4;
	static constexpr int values[value_count] = {10, 20, 30, 40};
	constexpr auto span = bstone::Span<const int>{values};
	constexpr auto subspan = span.get_subspan(1, 2);

	tester.check(
		subspan.get_data() == &values[1] &&
		subspan.get_size() == 2 &&
		subspan.get_data()[0] == values[1] &&
		subspan.get_data()[1] == values[2]);
}

// get_subspan(SpanInt) const
void test_ki8tkkkahutgp9nm()
{
	constexpr auto value_count = 4;
	static constexpr int values[value_count] = {10, 20, 30, 40};
	constexpr auto span = bstone::Span<const int>{values};
	constexpr auto subspan = span.get_subspan(2);

	tester.check(
		subspan.get_data() == &values[2] &&
		subspan.get_size() == 2 &&
		subspan.get_data()[0] == values[2] &&
		subspan.get_data()[1] == values[3]);
}

// operator[](SpanInt)
void test_ccsn7itkbetppctl()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::Span<const int>{values};
	tester.check(span.get_size() == value_count && span[1] == values[1]);
}

// swap(Span&)
void test_7uu7t14iuv1wi6xe()
{
	constexpr auto value_count = 2;
	static constexpr int values[value_count] = {10, 20};
	auto span_1 = bstone::Span<const int>{values};
	auto span = bstone::Span<const int>{};
	span.swap(span_1);
	tester.check(span.get_size() == value_count && span[0] == values[0] && span[1] == values[1]);
}

// ==========================================================================

// make_span(T*, SpanInt)
void test_klutjo3abbbdgb1q()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::make_span(values, value_count);

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// make_span(T*, SpanInt)
void test_5mtxi7yd3vi7rqaw()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::make_span(values);

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// ==========================================================================

// make_const_span(T*, SpanInt)
void test_822oeqj8jv4wtetl()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::make_const_span(values, value_count);

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// make_const_span(T*, SpanInt)
void test_9kk9lv5xylgwq7vl()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::make_const_span(values);

	tester.check(
		span.get_data() == values &&
		span.get_size() == value_count &&
		std::equal(values, &values[value_count], span.get_data()));
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_span();
		register_make_span();
		register_make_const_span();
	}

private:
	void register_span()
	{
		tester.register_test("Span#ue5mzqprtx1lm0q7", test_ue5mzqprtx1lm0q7);
		tester.register_test("Span#azyjatmoxj6k1qnh", test_azyjatmoxj6k1qnh);
		tester.register_test("Span#urx4i2uyw7z1k1di", test_urx4i2uyw7z1k1di);
		tester.register_test("Span#yxxe9qkpjtkmkwv7", test_yxxe9qkpjtkmkwv7);
		tester.register_test("Span#6u8ua35z28xhz00j", test_6u8ua35z28xhz00j);
		tester.register_test("Span#9qwtugkqkrq0snp9", test_9qwtugkqkrq0snp9);
		tester.register_test("Span#uhhdp4y73ufes1he", test_uhhdp4y73ufes1he);
		tester.register_test("Span#js5k7pyff5xm50gs", test_js5k7pyff5xm50gs);
		tester.register_test("Span#du81z5yyfuv6vy3b", test_du81z5yyfuv6vy3b);
		tester.register_test("Span#y1brl21aurhjqpb3", test_y1brl21aurhjqpb3);
		tester.register_test("Span#9g02a0e5r2iba386", test_9g02a0e5r2iba386);
		tester.register_test("Span#y82j43v4a54wo211", test_y82j43v4a54wo211);
		tester.register_test("Span#k93ls3eot86tjqyl", test_k93ls3eot86tjqyl);
		tester.register_test("Span#p9atqnjpm3ecfp2u", test_p9atqnjpm3ecfp2u);
		tester.register_test("Span#rrn85kwix9aioxj5", test_rrn85kwix9aioxj5);
		tester.register_test("Span#dgbwer4gkwz73ywp", test_dgbwer4gkwz73ywp);
		tester.register_test("Span#g1577nxmqwje2789", test_g1577nxmqwje2789);
		tester.register_test("Span#sq9gg9qdh2jeomnr", test_sq9gg9qdh2jeomnr);
		tester.register_test("Span#pjsw5n3iw8y9l8gm", test_pjsw5n3iw8y9l8gm);
		tester.register_test("Span#ki8tkkkahutgp9nm", test_ki8tkkkahutgp9nm);
		tester.register_test("Span#ccsn7itkbetppctl", test_ccsn7itkbetppctl);
		tester.register_test("Span#7uu7t14iuv1wi6xe", test_7uu7t14iuv1wi6xe);
	}

	void register_make_span()
	{
		tester.register_test("make_span#klutjo3abbbdgb1q", test_klutjo3abbbdgb1q);
		tester.register_test("make_span#5mtxi7yd3vi7rqaw", test_5mtxi7yd3vi7rqaw);
	}

	void register_make_const_span()
	{
		tester.register_test("make_span#822oeqj8jv4wtetl", test_822oeqj8jv4wtetl);
		tester.register_test("make_span#9kk9lv5xylgwq7vl", test_9kk9lv5xylgwq7vl);
	}
};

auto registrator = Registrator{};

} // namespace
