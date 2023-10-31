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
	constexpr auto bytes_count = value_count * static_cast<std::intptr_t>(sizeof(Type));
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

// make_const_span(Span<T>)
// Const.
void test_4mmnqq98rwfylh46()
{
	constexpr auto value_count = 3;
	const int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_span(values);
	const auto const_span = bstone::make_const_span(span);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		const_span.get_data() == values &&
		const_span.get_size() == value_count);
}

// make_const_span(Span<T>)
// Not const.
void test_dgoe56fk618qewwk()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_span(values);
	const auto const_span = bstone::make_const_span(span);
	using Span = decltype(span);

	tester.check(
		!std::is_const<Span::Item>::value &&
		const_span.get_data() == values &&
		const_span.get_size() == value_count);
}

// ==========================================================================

// make_octets_span(T*, SpanInt)
// Const.
void test_vz4841mkoy26qevn()
{
	constexpr auto value_count = 3;
	const int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_octets_span(values, value_count);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_octets_span(T*, SpanInt)
// Not const.
void test_cn0flxaqcrulubrz()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_octets_span(values, value_count);
	using Span = decltype(span);

	tester.check(
		!std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_octets_span(T (&)[TSize])
// Const.
void test_lbkl8witj32qwukg()
{
	constexpr auto value_count = 3;
	const int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_octets_span(values);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_octets_span(T (&)[TSize])
// Not const.
void test_4s142p3ilba894gr()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_octets_span(values);
	using Span = decltype(span);

	tester.check(
		!std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_octets_span(Span<T>)
// Const.
void test_piil1sd0ihg3940n()
{
	constexpr auto value_count = 3;
	static constexpr int values[value_count] = {10, 20, 30};
	constexpr auto span = bstone::make_span(values);
	const auto octets_span = bstone::make_octets_span(span);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		octets_span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		octets_span.get_size() == value_count * sizeof(int));
}

// make_octets_span(Span<T>)
// Not const.
void test_yxfuk39bm0ri0k2x()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_span(values);
	const auto octets_span = bstone::make_octets_span(span);
	using Span = decltype(span);

	tester.check(
		!std::is_const<Span::Item>::value &&
		octets_span.get_data() == reinterpret_cast<std::uint8_t*>(values) &&
		octets_span.get_size() == value_count * sizeof(int));
}

// ==========================================================================

// make_const_octets_span(T*, SpanInt)
// Const.
void test_xsswowtf98mfxyzg()
{
	constexpr auto value_count = 3;
	const int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_const_octets_span(values, value_count);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_const_octets_span(T*, SpanInt)
// Not const.
void test_ig10wut5vvtb1trm()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_const_octets_span(values, value_count);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_const_octets_span(T (&)[TSize])
// Const.
void test_apjutm6ccidvdbws()
{
	constexpr auto value_count = 3;
	const int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_const_octets_span(values);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_const_octets_span(T (&)[TSize])
// Not const.
void test_i7aizabfbdg2x2ee()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_const_octets_span(values);
	using Span = decltype(span);

	tester.check(
		std::is_const<Span::Item>::value &&
		span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		span.get_size() == value_count * sizeof(int));
}

// make_const_octets_span(Span<T>)
void test_jt0hrizy9066erco()
{
	constexpr auto value_count = 3;
	int values[value_count] = {10, 20, 30};
	const auto span = bstone::make_span(values);
	const auto octets_span = bstone::make_const_octets_span(span);
	using Span = decltype(octets_span);

	tester.check(
		std::is_const<Span::Item>::value &&
		octets_span.get_data() == reinterpret_cast<const std::uint8_t*>(values) &&
		octets_span.get_size() == value_count * sizeof(int));
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
		register_make_octets_span();
		register_make_const_octets_span();
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
		tester.register_test("make_const_span#822oeqj8jv4wtetl", test_822oeqj8jv4wtetl);
		tester.register_test("make_const_span#9kk9lv5xylgwq7vl", test_9kk9lv5xylgwq7vl);
		tester.register_test("make_const_span#4mmnqq98rwfylh46", test_4mmnqq98rwfylh46);
		tester.register_test("make_const_span#dgoe56fk618qewwk", test_dgoe56fk618qewwk);
	}

	void register_make_octets_span()
	{
		tester.register_test("make_octets_span#vz4841mkoy26qevn", test_vz4841mkoy26qevn);
		tester.register_test("make_octets_span#lbkl8witj32qwukg", test_lbkl8witj32qwukg);
		tester.register_test("make_octets_span#cn0flxaqcrulubrz", test_cn0flxaqcrulubrz);
		tester.register_test("make_octets_span#4s142p3ilba894gr", test_4s142p3ilba894gr);
		tester.register_test("make_octets_span#piil1sd0ihg3940n", test_piil1sd0ihg3940n);
		tester.register_test("make_octets_span#yxfuk39bm0ri0k2x", test_yxfuk39bm0ri0k2x);
	}

	void register_make_const_octets_span()
	{
		tester.register_test("make_const_octets_span#xsswowtf98mfxyzg", test_xsswowtf98mfxyzg);
		tester.register_test("make_const_octets_span#ig10wut5vvtb1trm", test_ig10wut5vvtb1trm);
		tester.register_test("make_const_octets_span#apjutm6ccidvdbws", test_apjutm6ccidvdbws);
		tester.register_test("make_const_octets_span#i7aizabfbdg2x2ee", test_i7aizabfbdg2x2ee);
		tester.register_test("make_const_octets_span#jt0hrizy9066erco", test_jt0hrizy9066erco);
	}
};

auto registrator = Registrator{};

} // namespace
