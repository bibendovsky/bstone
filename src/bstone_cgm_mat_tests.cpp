/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

/*
Computer Graphics Math:
Tests for a matrix
*/

#if !defined(NDEBUG)
#define BSTONE_CGM_MAT_TESTS
#endif

#if defined(BSTONE_CGM_MAT_TESTS)

#include <cmath>
#include <algorithm>
#include <array>
#include <new>
#include <type_traits>
#include "bstone_cgm_mat.h"
#include "bstone_exception.h"

namespace bstone {
namespace cgm {

namespace {


class MatTestsException : public Exception
{
public:
	explicit MatTestsException(const char* message)
		:
		Exception{"BSTONE_MAT_TESTS", message}
	{}
};

class MatTests
{
public:
	MatTests();
	MatTests(const MatTests&) = delete;
	MatTests& operator=(const MatTests&) = delete;

private:
	struct M4Tag {};

	template<Int M, Int N, typename T>
	class MatDeleter
	{
	public:
		using Mat = MatT<M, N, T>;

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

	[[noreturn]] void fail(const char* message)
	{
		throw new MatTestsException{message};
	}

	void check(bool condition, const char* message)
	{
		if (condition)
		{
			return;
		}

		fail(message);
	}

	template<Int M, Int N, typename T>
	static void make_mat(const std::array<T, M * N>& m, char (&storage)[M * N * sizeof(T)], const M4Tag)
	{
		new (storage) MatT<4, 4, T>(
			m[ 0], m[ 1], m[ 2], m[ 3],
			m[ 4], m[ 5], m[ 6], m[ 7],
			m[ 8], m[ 9], m[10], m[11],
			m[12], m[13], m[14], m[15]);
	}

	template<Int M, Int N, typename T>
	static void make_mat(const std::array<T, M * N>& m, char (&storage)[M * N * sizeof(T)])
	{
		using Tag = std::conditional_t<M == 4 && N == 4, M4Tag, void>;
		make_mat<M, N, T>(m, storage, Tag{});
	}

	template<Int M, Int N, typename T>
	static MatDeleter<M, N, T> make_mat_deleter(MatT<M, N, T>& m)
	{
		return MatDeleter<M, N, T>{m};
	}

	template<Int M, Int N, typename T>
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

	template<Int M, Int N, typename T>
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

	template<Int M, Int N, typename T>
	static bool are_equals_via_index(const std::array<T, M * N>& a, const MatT<M, N, T>& b)
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

	template<Int M, Int N, typename T>
	void test_ctor()
	{
		using Mat = MatT<M, N, T>;
		const auto m = Mat{};
		const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
		check(std::all_of(std::cbegin(am), std::cend(am), [](const typename Mat::Item& x) { return x == 0; }), "zgtmofkv");
	}

	template<Int M, Int N, typename T>
	void test_ctor_explicit_values()
	{
		using Mat = MatT<M, N, T>;
		const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
		char ms[sizeof(Mat)];
		auto& m = reinterpret_cast<Mat&>(ms);
		make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
		const auto md = make_mat_deleter(m);
		const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
		check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)), "d0uh15ki");
	}

	template<Int M, Int N, typename T>
	void test_copy_ctor()
	{
		using Mat = MatT<M, N, T>;
		const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
		char m0s[sizeof(Mat)];
		auto& m0 = reinterpret_cast<Mat&>(m0s);
		make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, m0s);
		const auto m0d = make_mat_deleter(m0);
		const auto m = m0;
		const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
		check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)), "mqiqc7xa");
	}

	template<Int M, Int N, typename T>
	void test_assignment_operator()
	{
		using Mat = MatT<M, N, T>;
		const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
		char m0s[sizeof(Mat)];
		auto& m0 = reinterpret_cast<Mat&>(m0s);
		make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, m0s);
		const auto m0d = make_mat_deleter(m0);
		auto m = Mat{};
		m = m0;
		const auto& am = reinterpret_cast<const typename Mat::Item(&)[Mat::item_count]>(m);
		check(std::equal(std::cbegin(am), std::cend(am), std::cbegin(a)), "qkdhxc8b");
	}

	template<Int M, Int N, typename T>
	void test_const_index_operator()
	{
		using Mat = MatT<M, N, T>;
		const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
		char ms[sizeof(Mat)];
		auto& m = reinterpret_cast<Mat&>(ms);
		make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
		const auto md = make_mat_deleter(m);
		check(are_equals_via_index(a, m), "1nnwph8j");
	}

	template<Int M, Int N, typename T>
	void test_index_operator()
	{
		using Mat = MatT<M, N, T>;
		const auto a = make_sequence_1<Mat::row_count, Mat::column_count, typename Mat::Item>();
		char ms[sizeof(Mat)];
		auto& m = reinterpret_cast<Mat&>(ms);
		make_mat<Mat::row_count, Mat::column_count, typename Mat::Item>(a, ms);
		const auto md = make_mat_deleter(m);

		for (auto i = decltype(Mat::item_count){}; i < Mat::item_count; ++i)
		{
			m[i] = a[i];
		}

		check(are_equals_via_index(a, m), "flpdorxc");
	}

	// ======================================================================

	template<Int M, Int N, typename T>
	void test()
	{
		test_ctor<M, N, T>();
		test_ctor_explicit_values<M, N, T>();
		test_copy_ctor<M, N, T>();
		test_assignment_operator<M, N, T>();
		test_const_index_operator<M, N, T>();
		test_index_operator<M, N, T>();
	}
};

MatTests::MatTests()
{
	test<4, 4, float>();

	test<4, 4, double>();
}

MatTests mat_tests{};

} // namespace

} // namespace cgm
} // namespace bstone

#endif // BSTONE_CGM_MAT_TESTS
