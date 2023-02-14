/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
Computer Graphics Math:
Tests for a vector
*/

#if !defined(NDEBUG)
#define BSTONE_CGM_VEC_TESTS
#endif

#if defined(BSTONE_CGM_VEC_TESTS)

#include <cmath>
#include <algorithm>
#include <array>
#include <new>
#include <type_traits>
#include "bstone_cgm_vec.h"
#include "bstone_exception.h"

namespace bstone {
namespace cgm {

namespace {


class VecTestsException : public Exception
{
public:
	explicit VecTestsException(const char* message)
		:
		Exception{"BSTONE_VEC_TESTS", message}
	{}
};

class VecTests
{
public:
	VecTests();
	VecTests(const VecTests&) = delete;
	VecTests& operator=(const VecTests&) = delete;

private:
	struct V2Tag {};
	struct V3Tag {};
	struct V4Tag {};

	template<Int N, typename T>
	class VecDeleter
	{
	public:
		using Vec = VecT<N, T>;

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

	[[noreturn]] void fail(const char* message)
	{
		throw new VecTestsException{message};
	}

	void check(bool condition, const char* message)
	{
		if (condition)
		{
			return;
		}

		fail(message);
	}

	template<Int N, typename T>
	static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V2Tag)
	{
		assert(storage != nullptr);
		new (storage) VecT<2, T>(v[0], v[1]);
	}

	template<Int N, typename T>
	static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V3Tag)
	{
		assert(storage != nullptr);
		new (storage) VecT<3, T>(v[0], v[1], v[2]);
	}

	template<Int N, typename T>
	static void make_vec(const std::array<T, N>& v, char (&storage)[N * sizeof(T)], const V4Tag)
	{
		assert(storage != nullptr);
		new (storage) VecT<4, T>(v[0], v[1], v[2], v[3]);
	}

	template<Int N, typename T>
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

		make_vec(v, storage, Tag{});
	}

	template<Int N, typename T>
	static VecDeleter<N, T> make_vec_deleter(VecT<N, T>& v)
	{
		return VecDeleter<N, T>{v};
	}

	template<Int N, typename T>
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

	template<Int N, typename T>
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

	template<Int N, typename T>
	static std::array<T, N> make_negated_sequence_1()
	{
		auto a = make_sequence_1<N, T>();

		for (auto& i : a)
		{
			i = -i;
		}

		return a;
	}

	template<Int N, typename T>
	static std::array<T, N> make_add_sequence_1_to_sequence_2()
	{
		auto a1 = make_sequence_1<N, T>();
		auto a2 = make_sequence_2<N, T>();
		auto b = std::array<T, N>{};
		std::transform(a1.cbegin(), a1.cend(), a2.cbegin(), b.begin(), std::plus<T>{});
		return b;
	}

	template<Int N, typename T>
	static std::array<T, N> make_sub_sequence_1_to_sequence_2()
	{
		auto a1 = make_sequence_1<N, T>();
		auto a2 = make_sequence_2<N, T>();
		auto b = std::array<T, N>{};
		std::transform(a1.cbegin(), a1.cend(), a2.cbegin(), b.begin(), std::minus<T>{});
		return b;
	}

	template<Int N, typename T>
	static std::array<T, N> make_multiplicated_sequence_1(T scalar)
	{
		auto a = make_sequence_1<N, T>();

		for (auto& i : a)
		{
			i *= scalar;
		}

		return a;
	}

	template<Int N, typename T>
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

	template<Int N, typename T>
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

	template<Int N, typename T>
	static T sqrt(const std::array<T, N>& x)
	{
		auto y = T{};

		for (const auto& i : x)
		{
			y += i * i;
		}

		return std::sqrt(y);
	}

	template<Int N, typename T>
	static bool are_equals_via_index(const std::array<T, N>& a, const VecT<N, T>& b)
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

	template<Int N, typename T>
	void test_ctor()
	{
		using Vec = VecT<N, T>;
		const auto v = Vec{};
		const auto& av = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v);
		check(std::all_of(std::cbegin(av), std::cend(av), [](const Vec::Item& x) { return x == 0; }), "1w3fjhlm");
	}

	template<Int N, typename T>
	void test_ctor_explicit_values()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		char vs[sizeof(Vec)];
		auto& v = reinterpret_cast<Vec&>(vs);
		make_vec(a, vs);
		const auto vd = make_vec_deleter(v);
		const auto& av = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v);
		check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)), "8hcgxo1f");
	}

	template<Int N, typename T>
	void test_copy_ctor()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		char v0s[sizeof(Vec)];
		auto& v0 = reinterpret_cast<Vec&>(v0s);
		make_vec(a, v0s);
		const auto v0d = make_vec_deleter(v0);
		const auto v = v0;
		const auto& av = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v);
		check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)), "purk597t");
	}

	template<Int N, typename T>
	void test_assignment_operator()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		char v0s[sizeof(Vec)];
		auto& v0 = reinterpret_cast<Vec&>(v0s);
		make_vec(a, v0s);
		const auto v0d = make_vec_deleter(v0);
		auto v = Vec{};
		v = v0;
		const auto& av = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v);
		check(std::equal(std::cbegin(av), std::cend(av), std::cbegin(a)), "6l0aq8mj");
	}

	template<Int N, typename T>
	void test_const_index_operator()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		char vs[sizeof(Vec)];
		auto& v = reinterpret_cast<Vec&>(vs);
		make_vec(a, vs);
		const auto vd = make_vec_deleter(v);
		check(are_equals_via_index(a, v), "makxzf0e");
	}

	template<Int N, typename T>
	void test_index_operator()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		char vs[sizeof(Vec)];
		auto& v = reinterpret_cast<Vec&>(vs);
		make_vec(a, vs);
		const auto vd = make_vec_deleter(v);

		for (auto i = decltype(Vec::item_count){}; i < Vec::item_count; ++i)
		{
			v[i] = a[i];
		}

		check(are_equals_via_index(a, v), "5bueqf35");
	}

	template<Int N, typename T>
	void test_get_magnitude()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();
		constexpr auto e = 1 / Vec::Item{100'000};
		assert(e > 0);
		const auto am = sqrt(a);
		char vs[sizeof(Vec)];
		auto& v = reinterpret_cast<Vec&>(vs);
		make_vec(a, vs);
		const auto vd = make_vec_deleter(v);
		const auto m = v.get_magnitude();
		check(std::abs(m - am) < e, "heqnw29b");
	}

	template<Int N, typename T>
	void test_equality_operator()
	{
		using Vec = VecT<N, T>;
		const auto a = make_sequence_1<Vec::item_count, Vec::Item>();

		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a, v1s);
		const auto v1d = make_vec_deleter(v1);

		char v2s[sizeof(Vec)];
		auto& v2 = reinterpret_cast<Vec&>(v2s);
		make_vec(a, v2s);
		const auto v2d = make_vec_deleter(v2);

		check(v1 == v2, "6oymv94n");
	}

	template<Int N, typename T>
	void test_inequality_operator()
	{
		using Vec = VecT<N, T>;

		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_sequence_2<Vec::item_count, Vec::Item>();
		char v2s[sizeof(Vec)];
		auto& v2 = reinterpret_cast<Vec&>(v2s);
		make_vec(a2, v2s);
		const auto v2d = make_vec_deleter(v2);

		check(v1 != v2, "7uk22jyc");
	}

	template<Int N, typename T>
	void test_unary_minus_operator()
	{
		using Vec = VecT<N, T>;

		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_negated_sequence_1<Vec::item_count, Vec::Item>();
		const auto v2 = -v1;

		const auto& av2 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v2);
		check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)), "srwo1yqh");
	}

	template<Int N, typename T>
	void test_addition_operator()
	{
		using Vec = VecT<N, T>;

		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_sequence_2<Vec::item_count, Vec::Item>();
		char v2s[sizeof(Vec)];
		auto& v2 = reinterpret_cast<Vec&>(v2s);
		make_vec(a2, v2s);
		const auto v2d = make_vec_deleter(v2);

		const auto a3 = make_add_sequence_1_to_sequence_2<Vec::item_count, Vec::Item>();
		const auto v3 = v1 + v2;
		const auto& av3 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v3);
		check(std::equal(std::cbegin(av3), std::cend(av3), std::cbegin(a3)), "i9xqnqd5");
	}

	template<Int N, typename T>
	void test_subtraction_operator()
	{
		using Vec = VecT<N, T>;

		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_sequence_2<Vec::item_count, Vec::Item>();
		char v2s[sizeof(Vec)];
		auto& v2 = reinterpret_cast<Vec&>(v2s);
		make_vec(a2, v2s);
		const auto v2d = make_vec_deleter(v2);

		const auto a3 = make_sub_sequence_1_to_sequence_2<Vec::item_count, Vec::Item>();
		const auto v3 = v1 - v2;
		const auto& av3 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v3);
		check(std::equal(std::cbegin(av3), std::cend(av3), std::cbegin(a3)), "2e8uimay");
	}

	template<Int N, typename T>
	void test_postfix_multiplication_operator_operator()
	{
		using Vec = VecT<N, T>;
		const auto scalar = T{3};
		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_multiplicated_sequence_1<Vec::item_count, Vec::Item>(scalar);
		const auto v2 = v1 * scalar;
		const auto& av2 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v2);
		check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)), "18gvxyw6");
	}

	template<Int N, typename T>
	void test_prefix_multiplication_operator()
	{
		using Vec = VecT<N, T>;
		const auto scalar = T{3};
		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_multiplicated_sequence_1<Vec::item_count, Vec::Item>(scalar);
		const auto v2 = scalar * v1;
		const auto& av2 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v2);
		check(std::equal(std::cbegin(av2), std::cend(av2), std::cbegin(a2)), "orko1yd5");
	}

	template<Int N, typename T>
	void test_dot()
	{
		using Vec = VecT<N, T>;

		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		const auto a2 = make_sequence_2<Vec::item_count, Vec::Item>();
		char v2s[sizeof(Vec)];
		auto& v2 = reinterpret_cast<Vec&>(v2s);
		make_vec(a2, v2s);
		const auto v2d = make_vec_deleter(v2);

		const auto d1 = calculate_dot_of_sequence_1_and_sequence_2<Vec::item_count, Vec::Item>();
		const auto d2 = dot(v1, v2);
		check(d1 == d2, "6hwwlhi2");
	}

	template<Int N, typename T>
	void test_normalize()
	{
		using Vec = VecT<N, T>;
		const auto a1 = make_sequence_1<Vec::item_count, Vec::Item>();
		char v1s[sizeof(Vec)];
		auto& v1 = reinterpret_cast<Vec&>(v1s);
		make_vec(a1, v1s);
		const auto v1d = make_vec_deleter(v1);

		constexpr auto e = 1 / Vec::Item{100'000};
		const auto a2 = make_normalized_sequence_1<Vec::item_count, Vec::Item>();
		const auto v2 = normalize(v1);
		const auto& av2 = reinterpret_cast<const Vec::Item(&)[Vec::item_count]>(v2);
		check(
			std::equal(
				std::cbegin(av2),
				std::cend(av2),
				std::cbegin(a2),
				[e](const Vec::Item& a, const Vec::Item& b)
				{
					return std::abs(a - b) < e;
				}),
			"dov8j4co");
	}

	// ======================================================================

	template<Int N, typename T>
	void test()
	{
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
	}
};

VecTests::VecTests()
{
	test<2, float>();
	test<3, float>();
	test<4, float>();

	test<2, double>();
	test<3, double>();
	test<4, double>();
}

VecTests vec_tests{};

} // namespace

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_VEC_TESTS
