#include <cassert>
#include <cmath>

#include <algorithm>
#include <array>
#include <new>
#include <type_traits>

#include "bstone_cgm_mat.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

struct M4Tag {};

template<std::intptr_t M, std::intptr_t N, typename T>
class MatDeleter
{
public:
	using Mat = bstone::cgm::Mat<M, N, T>;

	MatDeleter() = delete;

	explicit MatDeleter(Mat& m)
		:
		m_{&m}
	{}

	MatDeleter(const MatDeleter& rhs) = delete;

	MatDeleter(MatDeleter&& rhs) noexcept
	{
		std::swap(m_, rhs.m_);
	}

	MatDeleter& operator=(const MatDeleter& rhs) = delete;

	~MatDeleter()
	{
		if (m_ == nullptr)
		{
			return;
		}

		(*m_).~Mat();
	}

private:
	Mat* m_{};
};

template<std::intptr_t M, std::intptr_t N, typename T>
static void make_mat(const std::array<T, M * N>& m, char (&storage)[M * N * sizeof(T)], const M4Tag)
{
	new (storage) bstone::cgm::Mat<4, 4, T>(
		m[ 0], m[ 1], m[ 2], m[ 3],
		m[ 4], m[ 5], m[ 6], m[ 7],
		m[ 8], m[ 9], m[10], m[11],
		m[12], m[13], m[14], m[15]);
}

template<std::intptr_t M, std::intptr_t N, typename T>
static void make_mat(const std::array<T, M * N>& m, char (&storage)[M * N * sizeof(T)])
{
	using Tag = std::conditional_t<M == 4 && N == 4, M4Tag, void>;
	make_mat<M, N, T>(m, storage, Tag{});
}

template<std::intptr_t M, std::intptr_t N, typename T>
static MatDeleter<M, N, T> make_mat_deleter(bstone::cgm::Mat<M, N, T>& m)
{
	return MatDeleter<M, N, T>{m};
}

template<std::intptr_t M, std::intptr_t N, typename T>
static std::array<T, M * N> make_sequence_1()
{
	auto a = std::array<T, M * N>{};
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

template<std::intptr_t M, std::intptr_t N, typename T>
static std::array<T, M * N> make_sequence_2()
{
	auto a = std::array<T, M * N>{};
	auto value = 1;
	auto is_sign = true;

	for (auto& i : a)
	{
		i = static_cast<T>(is_sign ? -value : value);
		value += 2;
	}

	return a;
}

template<std::intptr_t M, std::intptr_t N, typename T>
static bool are_equals_via_index(const std::array<T, M * N>& a, const bstone::cgm::Mat<M, N, T>& b)
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

template<std::intptr_t M, std::intptr_t N, typename T>
void test_ctor()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto m = Mat{};
	const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
	tester.check(std::all_of(std::cbegin(am), std::cend(am), [](const typename Mat::Item& x) { return x == 0; }));
}

template<std::intptr_t M, std::intptr_t N, typename T>
void test_ctor_explicit_values()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
	char ms[sizeof(Mat)];
	auto& m = reinterpret_cast<Mat&>(ms);
	make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
	const auto md = make_mat_deleter(m);
	const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
	tester.check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)));
}

template<std::intptr_t M, std::intptr_t N, typename T>
void test_copy_ctor()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
	char m0s[sizeof(Mat)];
	auto& m0 = reinterpret_cast<Mat&>(m0s);
	make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, m0s);
	const auto m0d = make_mat_deleter(m0);
	const auto m = m0;
	const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
	tester.check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)));
}

template<std::intptr_t M, std::intptr_t N, typename T>
void test_assignment_operator()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
	char m0s[sizeof(Mat)];
	auto& m0 = reinterpret_cast<Mat&>(m0s);
	make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, m0s);
	const auto m0d = make_mat_deleter(m0);
	auto m = Mat{};
	m = m0;
	const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
	tester.check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)));
}

template<std::intptr_t M, std::intptr_t N, typename T>
void test_const_index_operator()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
	char ms[sizeof(Mat)];
	auto& m = reinterpret_cast<Mat&>(ms);
	make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
	const auto md = make_mat_deleter(m);
	tester.check(are_equals_via_index(a, m));
}

template<std::intptr_t M, std::intptr_t N, typename T>
void test_index_operator()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
	char ms[sizeof(Mat)];
	auto& m = reinterpret_cast<Mat&>(ms);
	make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
	const auto md = make_mat_deleter(m);

	for (auto i = decltype(Mat::item_count){}; i < Mat::item_count; ++i)
	{
		m[i] = a[i];
	}

	tester.check(are_equals_via_index(a, m));
}

// ======================================================================

/*
	test_ctor<M, N, T>();
	test_ctor_explicit_values<M, N, T>();
	test_copy_ctor<M, N, T>();
	test_assignment_operator<M, N, T>();
	test_const_index_operator<M, N, T>();
	test_index_operator<M, N, T>();
*/

// ==========================================================================

void test_ctor_44f()
{
	test_ctor<4, 4, float>();
}

void test_ctor_explicit_values_44f()
{
	test_ctor_explicit_values<4, 4, float>();
}

void test_copy_ctor_44f()
{
	test_copy_ctor<4, 4, float>();
}

void test_assignment_operator_44f()
{
	test_assignment_operator<4, 4, float>();
}

void test_const_index_operator_44f()
{
	test_const_index_operator<4, 4, float>();
}

void test_index_operator_44f()
{
	test_index_operator<4, 4, float>();
}

// ==========================================================================

void test_ctor_44d()
{
	test_ctor<4, 4, double>();
}

void test_ctor_explicit_values_44d()
{
	test_ctor_explicit_values<4, 4, double>();
}

void test_copy_ctor_44d()
{
	test_copy_ctor<4, 4, double>();
}

void test_assignment_operator_44d()
{
	test_assignment_operator<4, 4, double>();
}

void test_const_index_operator_44d()
{
	test_const_index_operator<4, 4, double>();
}

void test_index_operator_44d()
{
	test_index_operator<4, 4, double>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_test_44f();
		register_test_44d();
	}

private:
	void register_test_44f()
	{
		tester.register_test("cgm::Mat#4ag9fof3p4cklhe1", test_ctor_44f);
		tester.register_test("cgm::Mat#aq0051aftdgxbqc1", test_ctor_explicit_values_44f);
		tester.register_test("cgm::Mat#9ajzhghke0g75i09", test_copy_ctor_44f);
		tester.register_test("cgm::Mat#zobf72xgw9qpbepm", test_assignment_operator_44f);
		tester.register_test("cgm::Mat#r3x4lieb0dalgj2r", test_const_index_operator_44f);
		tester.register_test("cgm::Mat#gzh1qves11b74otx", test_index_operator_44f);
	}

	void register_test_44d()
	{
		tester.register_test("cgm::Mat#rqp3mnau2lwxzs8i", test_ctor_44d);
		tester.register_test("cgm::Mat#6aqe7tdicx8gdi8o", test_ctor_explicit_values_44d);
		tester.register_test("cgm::Mat#2fv9mtppn8urbwz8", test_copy_ctor_44d);
		tester.register_test("cgm::Mat#up86qvq5lga3ox9d", test_assignment_operator_44d);
		tester.register_test("cgm::Mat#jdwpbgx4nlq9pu60", test_const_index_operator_44d);
		tester.register_test("cgm::Mat#0rf50d77k55nz4td", test_index_operator_44d);
	}
};

auto registrator = Registrator{};

} // namespace
