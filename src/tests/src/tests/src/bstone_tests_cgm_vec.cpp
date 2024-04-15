#include <cassert>
#include <cmath>
#include <cstdint>

#include <algorithm>
#include <array>
#include <functional>
#include <new>
#include <type_traits>

#include "bstone_cgm_vec.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

struct V2Tag {};
struct V3Tag {};
struct V4Tag {};

template<std::intptr_t N, typename T>
class VecDeleter
{
public:
	using Vec = bstone::cgm::Vec<N, T>;

	VecDeleter() = delete;

	explicit VecDeleter(Vec& v)
		:
		v_{&v}
	{}

	VecDeleter(const VecDeleter& rhs) = delete;

	VecDeleter(VecDeleter&& rhs) noexcept
	{
		std::swap(v_, rhs.v_);
	}

	VecDeleter& operator=(const VecDeleter& rhs) = delete;

	~VecDeleter()
	{
		if (v_ == nullptr)
		{
			return;
		}

		(*v_).~Vec();
	}

private:
	Vec* v_{};
};

template<std::intptr_t N, typename T>
static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V2Tag)
{
	new (storage) bstone::cgm::Vec<2, T>(v[0], v[1]);
}

template<std::intptr_t N, typename T>
static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V3Tag)
{
	new (storage) bstone::cgm::Vec<3, T>(v[0], v[1], v[2]);
}

template<std::intptr_t N, typename T>
static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V4Tag)
{
	new (storage) bstone::cgm::Vec<4, T>(v[0], v[1], v[2], v[3]);
}

template<std::intptr_t N, typename T>
static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)])
{
	using Tag = std::conditional_t<
		N == 2,
		V2Tag,
		std::conditional_t<
			N == 3,
			V3Tag,
			std::conditional_t<
				N == 4,
				V4Tag,
				void>>>;

	make_vec<N, T>(v, storage, Tag{});
}

template<std::intptr_t N, typename T>
static VecDeleter<N, T> make_vec_deleter(bstone::cgm::Vec<N, T>& v)
{
	return VecDeleter<N, T>{v};
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_sequence_1()
{
	auto a = std::array<T, N>{};
	auto value = 1;
	auto is_sign = false;

	for (auto& i : a)
	{
		i = static_cast<T>(is_sign ? -value : value);
		++value;
		is_sign = !is_sign;
	}

	return a;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_sequence_2()
{
	auto a = std::array<T, N>{};
	auto value = 1;
	auto is_sign = true;

	for (auto& i : a)
	{
		i = static_cast<T>(is_sign ? -value : value);
		value += 2;
	}

	return a;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_negated_sequence_1()
{
	auto a = make_sequence_1<N, T>();

	for (auto& i : a)
	{
		i = -i;
	}

	return a;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_add_sequence_1_to_sequence_2()
{
	auto a1 = make_sequence_1<N, T>();
	auto a2 = make_sequence_2<N, T>();
	auto b = std::array<T, N>{};
	std::transform(a1.cbegin(), a1.cend(), a2.cbegin(), b.begin(), std::plus<T>{});
	return b;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_sub_sequence_1_to_sequence_2()
{
	auto a1 = make_sequence_1<N, T>();
	auto a2 = make_sequence_2<N, T>();
	auto b = std::array<T, N>{};
	std::transform(a1.cbegin(), a1.cend(), a2.cbegin(), b.begin(), std::minus<T>{});
	return b;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_multiplicated_sequence_1(T scalar)
{
	auto a = make_sequence_1<N, T>();

	for (auto& i : a)
	{
		i *= scalar;
	}

	return a;
}

template<std::intptr_t N, typename T>
static T calculate_dot_of_sequence_1_and_sequence_2()
{
	auto a1 = make_sequence_1<N, T>();
	auto a2 = make_sequence_2<N, T>();
	auto r = T{};

	for (auto i = decltype(N){}; i < N; ++i)
	{
		r += a1[i] * a2[i];
	}

	return r;
}

template<std::intptr_t N, typename T>
static std::array<T, N> make_normalized_sequence_1()
{
	auto a1 = make_sequence_1<N, T>();
	auto squared_magnitude = T{};

	for (const auto& i : a1)
	{
		squared_magnitude += i * i;
	}

	const auto r_magnitude = 1 / std::sqrt(squared_magnitude);

	auto b = a1;

	for (auto& i : b)
	{
		i *= r_magnitude;
	}

	return b;
}

template<std::intptr_t N, typename T>
static T sqrt(const std::array<T, N>& x)
{
	auto y = T{};

	for (const auto& i : x)
	{
		y += i * i;
	}

	return std::sqrt(y);
}

template<std::intptr_t N, typename T>
static bool are_equals_via_index(const std::array<T, N>& a, const bstone::cgm::Vec<N, T>& b)
{
	assert(static_cast<decltype(b.item_count)>(a.size()) == b.item_count);

	for (auto i = decltype(b.item_count){}; i < b.item_count; ++i)
	{
		if (a[i] != b[i])
		{
			return false;
		}
	}

	return true;
}

template<std::intptr_t N, typename T>
void test_ctor()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto v = Vec{};
	const auto& av = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v);

	tester.check(std::all_of(
		std::cbegin(av),
			std::cend(av),
			[](const typename Vec::Item& x) { return x == 0; }));
}

template<std::intptr_t N, typename T>
void test_ctor_explicit_values()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char vs[sizeof(Vec)];
	auto& v = reinterpret_cast<Vec&>(vs);
	make_vec<N, T>(a, vs);
	const auto vd = make_vec_deleter(v);
	const auto& av = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v);
	tester.check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)));
}

template<std::intptr_t N, typename T>
void test_copy_ctor()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v0s[sizeof(Vec)];
	auto& v0 = reinterpret_cast<Vec&>(v0s);
	make_vec<N, T>(a, v0s);
	const auto v0d = make_vec_deleter(v0);
	const auto v = v0;
	const auto& av = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v);
	tester.check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)));
}

template<std::intptr_t N, typename T>
void test_assignment_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v0s[sizeof(Vec)];
	auto& v0 = reinterpret_cast<Vec&>(v0s);
	make_vec<N, T>(a, v0s);
	const auto v0d = make_vec_deleter(v0);
	auto v = Vec{};
	v = v0;
	const auto& av = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v);
	tester.check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)));
}

template<std::intptr_t N, typename T>
void test_const_index_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char vs[sizeof(Vec)];
	auto& v = reinterpret_cast<Vec&>(vs);
	make_vec<N, T>(a, vs);
	const auto vd = make_vec_deleter(v);
	tester.check(are_equals_via_index<N, T>(a, v));
}

template<std::intptr_t N, typename T>
void test_index_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char vs[sizeof(Vec)];
	auto& v = reinterpret_cast<Vec&>(vs);
	make_vec<N, T>(a, vs);
	const auto vd = make_vec_deleter(v);

	for (auto i = decltype(Vec::item_count){}; i < Vec::item_count; ++i)
	{
		v[i] = a[i];
	}

	tester.check(are_equals_via_index<N, T>(a, v));
}

template<std::intptr_t N, typename T>
void test_get_magnitude()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();
	constexpr auto e = 1 / typename Vec::Item{100'000};
	assert(e > 0);
	const auto am = sqrt<N, T>(a);
	char vs[sizeof(Vec)];
	auto& v = reinterpret_cast<Vec&>(vs);
	make_vec<N, T>(a, vs);
	const auto vd = make_vec_deleter(v);
	const auto m = get_magnitude(v);
	tester.check(std::abs(m - am) < e);
}

template<std::intptr_t N, typename T>
void test_equality_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a = make_sequence_1<Vec::item_count, typename Vec::Item>();

	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a, v1s);
	const auto v1d = make_vec_deleter(v1);

	char v2s[sizeof(Vec)];
	auto& v2 = reinterpret_cast<Vec&>(v2s);
	make_vec<N, T>(a, v2s);
	const auto v2d = make_vec_deleter(v2);

	tester.check(v1 == v2);
}

template<std::intptr_t N, typename T>
void test_inequality_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;

	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_sequence_2<Vec::item_count, typename Vec::Item>();
	char v2s[sizeof(Vec)];
	auto& v2 = reinterpret_cast<Vec&>(v2s);
	make_vec<N, T>(a2, v2s);
	const auto v2d = make_vec_deleter(v2);

	tester.check(v1 != v2);
}

template<std::intptr_t N, typename T>
void test_unary_minus_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;

	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_negated_sequence_1<Vec::item_count, typename Vec::Item>();
	const auto v2 = -v1;

	const auto& av2 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v2);
	tester.check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)));
}

template<std::intptr_t N, typename T>
void test_addition_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;

	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_sequence_2<Vec::item_count, typename Vec::Item>();
	char v2s[sizeof(Vec)];
	auto& v2 = reinterpret_cast<Vec&>(v2s);
	make_vec<N, T>(a2, v2s);
	const auto v2d = make_vec_deleter(v2);

	const auto a3 = make_add_sequence_1_to_sequence_2<Vec::item_count, typename Vec::Item>();
	const auto v3 = v1 + v2;
	const auto& av3 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v3);
	tester.check(std::equal(std::cbegin(av3), std::cend(av3), std::cbegin(a3)));
}

template<std::intptr_t N, typename T>
void test_subtraction_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;

	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_sequence_2<Vec::item_count, typename Vec::Item>();
	char v2s[sizeof(Vec)];
	auto& v2 = reinterpret_cast<Vec&>(v2s);
	make_vec<N, T>(a2, v2s);
	const auto v2d = make_vec_deleter(v2);

	const auto a3 = make_sub_sequence_1_to_sequence_2<Vec::item_count, typename Vec::Item>();
	const auto v3 = v1 - v2;
	const auto& av3 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v3);
	tester.check(std::equal(std::cbegin(av3), std::cend(av3), std::cbegin(a3)));
}

template<std::intptr_t N, typename T>
void test_postfix_multiplication_operator_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto scalar = T{3};
	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_multiplicated_sequence_1<Vec::item_count, typename Vec::Item>(scalar);
	const auto v2 = v1 * scalar;
	const auto& av2 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v2);
	tester.check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)));
}

template<std::intptr_t N, typename T>
void test_prefix_multiplication_operator()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto scalar = T{3};
	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_multiplicated_sequence_1<Vec::item_count, typename Vec::Item>(scalar);
	const auto v2 = scalar * v1;
	const auto& av2 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v2);
	tester.check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)));
}

template<std::intptr_t N, typename T>
void test_dot()
{
	using Vec = bstone::cgm::Vec<N, T>;

	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_sequence_2<Vec::item_count, typename Vec::Item>();
	char v2s[sizeof(Vec)];
	auto& v2 = reinterpret_cast<Vec&>(v2s);
	make_vec<N, T>(a2, v2s);
	const auto v2d = make_vec_deleter(v2);

	const auto d1 = calculate_dot_of_sequence_1_and_sequence_2<Vec::item_count, typename Vec::Item>();
	const auto d2 = dot(v1, v2);
	tester.check(d1 == d2);
}

template<std::intptr_t N, typename T>
void test_normalize()
{
	using Vec = bstone::cgm::Vec<N, T>;
	const auto a1 = make_sequence_1<Vec::item_count, typename Vec::Item>();
	char v1s[sizeof(Vec)];
	auto& v1 = reinterpret_cast<Vec&>(v1s);
	make_vec<N, T>(a1, v1s);
	const auto v1d = make_vec_deleter(v1);

	const auto a2 = make_normalized_sequence_1<Vec::item_count, typename Vec::Item>();
	const auto v2 = normalize(v1);
	const auto& av2 = reinterpret_cast<const typename Vec::Item(&)[Vec::item_count]>(v2);
	tester.check(
		std::equal(
			std::cbegin(av2),
			std::cend(av2),
			std::cbegin(a2),
			[](const typename Vec::Item& a, const typename Vec::Item& b)
			{
				constexpr auto e = 1 / typename Vec::Item{100'000};
				return std::abs(a - b) < e;
			}));
}

// ======================================================================

/*
	test_ctor<N, T>();
	test_ctor_explicit_values<N, T>();
	test_copy_ctor<N, T>();
	test_assignment_operator<N, T>();
	test_const_index_operator<N, T>();
	test_index_operator<N, T>();
	test_get_magnitude<N, T>();
	test_equality_operator<N, T>();
	test_inequality_operator<N, T>();
	test_unary_minus_operator<N, T>();
	test_addition_operator<N, T>();
	test_subtraction_operator<N, T>();
	test_postfix_multiplication_operator_operator<N, T>();
	test_prefix_multiplication_operator<N, T>();
	test_dot<N, T>();
	test_normalize<N, T>();
*/

// ==========================================================================

void test_ctor_2f()
{
	test_ctor<2, float>();
}

void test_ctor_explicit_values_2f()
{
	test_ctor_explicit_values<2, float>();
}

void test_copy_ctor_2f()
{
	test_copy_ctor<2, float>();
}

void test_assignment_operator_2f()
{
	test_assignment_operator<2, float>();
}

void test_const_index_operator_2f()
{
	test_const_index_operator<2, float>();
}

void test_index_operator_2f()
{
	test_index_operator<2, float>();
}

void test_get_magnitude_2f()
{
	test_get_magnitude<2, float>();
}

void test_equality_operator_2f()
{
	test_equality_operator<2, float>();
}

void test_inequality_operator_2f()
{
	test_inequality_operator<2, float>();
}

void test_unary_minus_operator_2f()
{
	test_unary_minus_operator<2, float>();
}

void test_addition_operator_2f()
{
	test_addition_operator<2, float>();
}

void test_subtraction_operator_2f()
{
	test_subtraction_operator<2, float>();
}

void test_postfix_multiplication_operator_operator_2f()
{
	test_postfix_multiplication_operator_operator<2, float>();
}

void test_prefix_multiplication_operator_2f()
{
	test_prefix_multiplication_operator<2, float>();
}

void test_dot_2f()
{
	test_dot<2, float>();
}

void test_normalize_2f()
{
	test_normalize<2, float>();
}

// ==========================================================================

void test_ctor_3f()
{
	test_ctor<3, float>();
}

void test_ctor_explicit_values_3f()
{
	test_ctor_explicit_values<3, float>();
}

void test_copy_ctor_3f()
{
	test_copy_ctor<3, float>();
}

void test_assignment_operator_3f()
{
	test_assignment_operator<3, float>();
}

void test_const_index_operator_3f()
{
	test_const_index_operator<3, float>();
}

void test_index_operator_3f()
{
	test_index_operator<3, float>();
}

void test_get_magnitude_3f()
{
	test_get_magnitude<3, float>();
}

void test_equality_operator_3f()
{
	test_equality_operator<3, float>();
}

void test_inequality_operator_3f()
{
	test_inequality_operator<3, float>();
}

void test_unary_minus_operator_3f()
{
	test_unary_minus_operator<3, float>();
}

void test_addition_operator_3f()
{
	test_addition_operator<3, float>();
}

void test_subtraction_operator_3f()
{
	test_subtraction_operator<3, float>();
}

void test_postfix_multiplication_operator_operator_3f()
{
	test_postfix_multiplication_operator_operator<3, float>();
}

void test_prefix_multiplication_operator_3f()
{
	test_prefix_multiplication_operator<3, float>();
}

void test_dot_3f()
{
	test_dot<3, float>();
}

void test_normalize_3f()
{
	test_normalize<3, float>();
}

// ==========================================================================

void test_ctor_4f()
{
	test_ctor<4, float>();
}

void test_ctor_explicit_values_4f()
{
	test_ctor_explicit_values<4, float>();
}

void test_copy_ctor_4f()
{
	test_copy_ctor<4, float>();
}

void test_assignment_operator_4f()
{
	test_assignment_operator<4, float>();
}

void test_const_index_operator_4f()
{
	test_const_index_operator<4, float>();
}

void test_index_operator_4f()
{
	test_index_operator<4, float>();
}

void test_get_magnitude_4f()
{
	test_get_magnitude<4, float>();
}

void test_equality_operator_4f()
{
	test_equality_operator<4, float>();
}

void test_inequality_operator_4f()
{
	test_inequality_operator<4, float>();
}

void test_unary_minus_operator_4f()
{
	test_unary_minus_operator<4, float>();
}

void test_addition_operator_4f()
{
	test_addition_operator<4, float>();
}

void test_subtraction_operator_4f()
{
	test_subtraction_operator<4, float>();
}

void test_postfix_multiplication_operator_operator_4f()
{
	test_postfix_multiplication_operator_operator<4, float>();
}

void test_prefix_multiplication_operator_4f()
{
	test_prefix_multiplication_operator<4, float>();
}

void test_dot_4f()
{
	test_dot<4, float>();
}

void test_normalize_4f()
{
	test_normalize<4, float>();
}

// ==========================================================================

void test_ctor_2d()
{
	test_ctor<2, double>();
}

void test_ctor_explicit_values_2d()
{
	test_ctor_explicit_values<2, double>();
}

void test_copy_ctor_2d()
{
	test_copy_ctor<2, double>();
}

void test_assignment_operator_2d()
{
	test_assignment_operator<2, double>();
}

void test_const_index_operator_2d()
{
	test_const_index_operator<2, double>();
}

void test_index_operator_2d()
{
	test_index_operator<2, double>();
}

void test_get_magnitude_2d()
{
	test_get_magnitude<2, double>();
}

void test_equality_operator_2d()
{
	test_equality_operator<2, double>();
}

void test_inequality_operator_2d()
{
	test_inequality_operator<2, double>();
}

void test_unary_minus_operator_2d()
{
	test_unary_minus_operator<2, double>();
}

void test_addition_operator_2d()
{
	test_addition_operator<2, double>();
}

void test_subtraction_operator_2d()
{
	test_subtraction_operator<2, double>();
}

void test_postfix_multiplication_operator_operator_2d()
{
	test_postfix_multiplication_operator_operator<2, double>();
}

void test_prefix_multiplication_operator_2d()
{
	test_prefix_multiplication_operator<2, double>();
}

void test_dot_2d()
{
	test_dot<2, double>();
}

void test_normalize_2d()
{
	test_normalize<2, double>();
}

// ==========================================================================

void test_ctor_3d()
{
	test_ctor<3, double>();
}

void test_ctor_explicit_values_3d()
{
	test_ctor_explicit_values<3, double>();
}

void test_copy_ctor_3d()
{
	test_copy_ctor<3, double>();
}

void test_assignment_operator_3d()
{
	test_assignment_operator<3, double>();
}

void test_const_index_operator_3d()
{
	test_const_index_operator<3, double>();
}

void test_index_operator_3d()
{
	test_index_operator<3, double>();
}

void test_get_magnitude_3d()
{
	test_get_magnitude<3, double>();
}

void test_equality_operator_3d()
{
	test_equality_operator<3, double>();
}

void test_inequality_operator_3d()
{
	test_inequality_operator<3, double>();
}

void test_unary_minus_operator_3d()
{
	test_unary_minus_operator<3, double>();
}

void test_addition_operator_3d()
{
	test_addition_operator<3, double>();
}

void test_subtraction_operator_3d()
{
	test_subtraction_operator<3, double>();
}

void test_postfix_multiplication_operator_operator_3d()
{
	test_postfix_multiplication_operator_operator<3, double>();
}

void test_prefix_multiplication_operator_3d()
{
	test_prefix_multiplication_operator<3, double>();
}

void test_dot_3d()
{
	test_dot<3, double>();
}

void test_normalize_3d()
{
	test_normalize<3, double>();
}

// ==========================================================================

void test_ctor_4d()
{
	test_ctor<4, double>();
}

void test_ctor_explicit_values_4d()
{
	test_ctor_explicit_values<4, double>();
}

void test_copy_ctor_4d()
{
	test_copy_ctor<4, double>();
}

void test_assignment_operator_4d()
{
	test_assignment_operator<4, double>();
}

void test_const_index_operator_4d()
{
	test_const_index_operator<4, double>();
}

void test_index_operator_4d()
{
	test_index_operator<4, double>();
}

void test_get_magnitude_4d()
{
	test_get_magnitude<4, double>();
}

void test_equality_operator_4d()
{
	test_equality_operator<4, double>();
}

void test_inequality_operator_4d()
{
	test_inequality_operator<4, double>();
}

void test_unary_minus_operator_4d()
{
	test_unary_minus_operator<4, double>();
}

void test_addition_operator_4d()
{
	test_addition_operator<4, double>();
}

void test_subtraction_operator_4d()
{
	test_subtraction_operator<4, double>();
}

void test_postfix_multiplication_operator_operator_4d()
{
	test_postfix_multiplication_operator_operator<4, double>();
}

void test_prefix_multiplication_operator_4d()
{
	test_prefix_multiplication_operator<4, double>();
}

void test_dot_4d()
{
	test_dot<4, double>();
}

void test_normalize_4d()
{
	test_normalize<4, double>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_test_2f();
		register_test_3f();
		register_test_4f();

		register_test_2d();
		register_test_3d();
		register_test_4d();
	}

private:
	void register_test_2f()
	{
		tester.register_test("cgm::Vec#l460vhufwwne2vfa", test_ctor_2f);
		tester.register_test("cgm::Vec#d497gq5qzb0tqugh", test_ctor_explicit_values_2f);
		tester.register_test("cgm::Vec#spumm8qusm6hyc51", test_copy_ctor_2f);
		tester.register_test("cgm::Vec#hgduvp36nuu2zayc", test_assignment_operator_2f);
		tester.register_test("cgm::Vec#arxmz8dxo6d00ldp", test_const_index_operator_2f);
		tester.register_test("cgm::Vec#p4vdg8plsuzz7p6t", test_index_operator_2f);
		tester.register_test("cgm::Vec#slb9r1oh4buflgud", test_get_magnitude_2f);
		tester.register_test("cgm::Vec#4gi18naxhgzxo1yy", test_equality_operator_2f);
		tester.register_test("cgm::Vec#o1i9jpx0mw5bqfvt", test_inequality_operator_2f);
		tester.register_test("cgm::Vec#4c19t542c04f7t2n", test_unary_minus_operator_2f);
		tester.register_test("cgm::Vec#h3pw9qhek8d6qfp8", test_addition_operator_2f);
		tester.register_test("cgm::Vec#t5v7wnjryiqe0vd9", test_subtraction_operator_2f);
		tester.register_test("cgm::Vec#a45k01catfcljir8", test_postfix_multiplication_operator_operator_2f);
		tester.register_test("cgm::Vec#wh4n2q7dubouas6x", test_prefix_multiplication_operator_2f);
		tester.register_test("cgm::Vec#lynnic3613d80c55", test_dot_2f);
		tester.register_test("cgm::Vec#ak6hf4jby562657r", test_normalize_2f);
	}

	void register_test_3f()
	{
		tester.register_test("cgm::Vec#8cpq3n8yz16pdb3d", test_ctor_3f);
		tester.register_test("cgm::Vec#s6vvnd389nhl6ine", test_ctor_explicit_values_3f);
		tester.register_test("cgm::Vec#zko9ecq825fjpqn6", test_copy_ctor_3f);
		tester.register_test("cgm::Vec#iulsntztrrx3ml3b", test_assignment_operator_3f);
		tester.register_test("cgm::Vec#l7bfc4v8x4vhkl9d", test_const_index_operator_3f);
		tester.register_test("cgm::Vec#3qg4vydllb6z5mtk", test_index_operator_3f);
		tester.register_test("cgm::Vec#g1a8yrbs6bxi5r4g", test_get_magnitude_3f);
		tester.register_test("cgm::Vec#2a06yy8ap5enum91", test_equality_operator_3f);
		tester.register_test("cgm::Vec#mysxa3bcrgcs5xc3", test_inequality_operator_3f);
		tester.register_test("cgm::Vec#0cikkdigh933nrjd", test_unary_minus_operator_3f);
		tester.register_test("cgm::Vec#lewnohj9gnnye0ud", test_addition_operator_3f);
		tester.register_test("cgm::Vec#3ltxw2jxosrtiap5", test_subtraction_operator_3f);
		tester.register_test("cgm::Vec#xeqs46ehrzadssxa", test_postfix_multiplication_operator_operator_3f);
		tester.register_test("cgm::Vec#olxnwla8k9bmd401", test_prefix_multiplication_operator_3f);
		tester.register_test("cgm::Vec#l460vhufwwne2vfa", test_dot_3f);
		tester.register_test("cgm::Vec#d497gq5qzb0tqugh", test_normalize_3f);
	}

	void register_test_4f()
	{
		tester.register_test("cgm::Vec#spumm8qusm6hyc51", test_ctor_4f);
		tester.register_test("cgm::Vec#hgduvp36nuu2zayc", test_ctor_explicit_values_4f);
		tester.register_test("cgm::Vec#arxmz8dxo6d00ldp", test_copy_ctor_4f);
		tester.register_test("cgm::Vec#p4vdg8plsuzz7p6t", test_assignment_operator_4f);
		tester.register_test("cgm::Vec#slb9r1oh4buflgud", test_const_index_operator_4f);
		tester.register_test("cgm::Vec#4gi18naxhgzxo1yy", test_index_operator_4f);
		tester.register_test("cgm::Vec#o1i9jpx0mw5bqfvt", test_get_magnitude_4f);
		tester.register_test("cgm::Vec#4c19t542c04f7t2n", test_equality_operator_4f);
		tester.register_test("cgm::Vec#h3pw9qhek8d6qfp8", test_inequality_operator_4f);
		tester.register_test("cgm::Vec#t5v7wnjryiqe0vd9", test_unary_minus_operator_4f);
		tester.register_test("cgm::Vec#a45k01catfcljir8", test_addition_operator_4f);
		tester.register_test("cgm::Vec#wh4n2q7dubouas6x", test_subtraction_operator_4f);
		tester.register_test("cgm::Vec#lynnic3613d80c55", test_postfix_multiplication_operator_operator_4f);
		tester.register_test("cgm::Vec#ak6hf4jby562657r", test_prefix_multiplication_operator_4f);
		tester.register_test("cgm::Vec#8cpq3n8yz16pdb3d", test_dot_4f);
		tester.register_test("cgm::Vec#s6vvnd389nhl6ine", test_normalize_4f);
	}

	void register_test_2d()
	{
		tester.register_test("cgm::Vec#zko9ecq825fjpqn6", test_ctor_2d);
		tester.register_test("cgm::Vec#iulsntztrrx3ml3b", test_ctor_explicit_values_2d);
		tester.register_test("cgm::Vec#l7bfc4v8x4vhkl9d", test_copy_ctor_2d);
		tester.register_test("cgm::Vec#3qg4vydllb6z5mtk", test_assignment_operator_2d);
		tester.register_test("cgm::Vec#g1a8yrbs6bxi5r4g", test_const_index_operator_2d);
		tester.register_test("cgm::Vec#2a06yy8ap5enum91", test_index_operator_2d);
		tester.register_test("cgm::Vec#mysxa3bcrgcs5xc3", test_get_magnitude_2d);
		tester.register_test("cgm::Vec#0cikkdigh933nrjd", test_equality_operator_2d);
		tester.register_test("cgm::Vec#lewnohj9gnnye0ud", test_inequality_operator_2d);
		tester.register_test("cgm::Vec#3ltxw2jxosrtiap5", test_unary_minus_operator_2d);
		tester.register_test("cgm::Vec#xeqs46ehrzadssxa", test_addition_operator_2d);
		tester.register_test("cgm::Vec#olxnwla8k9bmd401", test_subtraction_operator_2d);
		tester.register_test("cgm::Vec#1jhyly44lj1vn6nk", test_postfix_multiplication_operator_operator_2d);
		tester.register_test("cgm::Vec#gmj086ymvbnk21xi", test_prefix_multiplication_operator_2d);
		tester.register_test("cgm::Vec#267yy3axhxtxpb87", test_dot_2d);
		tester.register_test("cgm::Vec#lqa5ylb5io2bmje7", test_normalize_2d);
	}

	void register_test_3d()
	{
		tester.register_test("cgm::Vec#7rr9n8sh7i94a35g", test_ctor_3d);
		tester.register_test("cgm::Vec#tiwlhgh3306tb8x9", test_ctor_explicit_values_3d);
		tester.register_test("cgm::Vec#6a7n2z5qt5y8vdsf", test_copy_ctor_3d);
		tester.register_test("cgm::Vec#84gh5htforbbrd93", test_assignment_operator_3d);
		tester.register_test("cgm::Vec#oin17ybjjw6lkxms", test_const_index_operator_3d);
		tester.register_test("cgm::Vec#5lldoh7e5595bkyg", test_index_operator_3d);
		tester.register_test("cgm::Vec#mrfojn8tgzao911s", test_get_magnitude_3d);
		tester.register_test("cgm::Vec#s5610v7hszg201fs", test_equality_operator_3d);
		tester.register_test("cgm::Vec#5r53zcufppxab8rl", test_inequality_operator_3d);
		tester.register_test("cgm::Vec#by28o59b1c8fg7rk", test_unary_minus_operator_3d);
		tester.register_test("cgm::Vec#nohu4kdsee57cj3i", test_addition_operator_3d);
		tester.register_test("cgm::Vec#1z0cphiost6ayudf", test_subtraction_operator_3d);
		tester.register_test("cgm::Vec#q6g9d6zngl4x6pq4", test_postfix_multiplication_operator_operator_3d);
		tester.register_test("cgm::Vec#msb1cdoviqos33m9", test_prefix_multiplication_operator_3d);
		tester.register_test("cgm::Vec#doxzm6hmuvci0uyw", test_dot_3d);
		tester.register_test("cgm::Vec#ap1rgr65v5ffq13u", test_normalize_3d);
	}

	void register_test_4d()
	{
		tester.register_test("cgm::Vec#6q2co5jrpx665ufx", test_ctor_4d);
		tester.register_test("cgm::Vec#277m2aum95skbg7z", test_ctor_explicit_values_4d);
		tester.register_test("cgm::Vec#5x1mnxd3fhkv99w4", test_copy_ctor_4d);
		tester.register_test("cgm::Vec#sw9jg6acqtskmxlv", test_assignment_operator_4d);
		tester.register_test("cgm::Vec#tov64xqpazcunma2", test_const_index_operator_4d);
		tester.register_test("cgm::Vec#kf8se9ho6e5sma0a", test_index_operator_4d);
		tester.register_test("cgm::Vec#52tpioxzocki8z90", test_get_magnitude_4d);
		tester.register_test("cgm::Vec#z2enl2ga9zrgspp4", test_equality_operator_4d);
		tester.register_test("cgm::Vec#71un4ngm80eavo7n", test_inequality_operator_4d);
		tester.register_test("cgm::Vec#f5a2k3mj8yynrd34", test_unary_minus_operator_4d);
		tester.register_test("cgm::Vec#hawatoytnyfjsh0y", test_addition_operator_4d);
		tester.register_test("cgm::Vec#xblp6v1m5y4alh7f", test_subtraction_operator_4d);
		tester.register_test("cgm::Vec#7iumvi0xat72q0lp", test_postfix_multiplication_operator_operator_4d);
		tester.register_test("cgm::Vec#cp7vnol662n736jc", test_prefix_multiplication_operator_4d);
		tester.register_test("cgm::Vec#0vyrzzbcrkq51bnt", test_dot_4d);
		tester.register_test("cgm::Vec#0b6m8iqt2fg9zuzj", test_normalize_4d);
	}
};

auto registrator = Registrator{};

} // namespace
