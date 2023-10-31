#include <cmath>

#include "bstone_tester.h"

#include "bstone_cgm_clip_space.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

template<typename T>
constexpr T abs(T x) noexcept
{
	return x < 0 ? -x : x;
}

template<typename T>
constexpr bool are_mat4_equal(
	const bstone::cgm::Mat<4, 4, T>& a,
	const bstone::cgm::Mat<4, 4, T>& b, T epsilon) noexcept
{
	for (auto i = 0; i < 16; ++i)
	{
		if (abs(a[i] - b[i]) > epsilon)
		{
			return false;
		}
	}

	return true;
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_make_ortho_rh_n1p1()
{
	using Mat = bstone::cgm::Mat<M, N, T>;

	constexpr auto ref_l = static_cast<T>(-10);
	constexpr auto ref_r = static_cast<T>(630);
	constexpr auto ref_b = static_cast<T>(-5);
	constexpr auto ref_t = static_cast<T>(475);
	constexpr auto ref_n = static_cast<T>(0.5);
	constexpr auto ref_f = static_cast<T>(25);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(0.0031250000465661287),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0.0041666668839752674),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(-0.081632651388645172),
		static_cast<T>(0),

		static_cast<T>(-0.96875),
		static_cast<T>(-0.97916674613952637),
		static_cast<T>(-1.0408163070678711),
		static_cast<T>(1),
	};

	constexpr auto epsilon = 1 / static_cast<T>(100'000);
	constexpr auto value = bstone::cgm::make_ortho_rh_n1p1(ref_l, ref_r, ref_b, ref_t, ref_n, ref_f);

	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void make_perspective_vfov_rh_n1p1()
{
	using Mat = bstone::cgm::Mat<M, N, T>;

	constexpr auto ref_v = static_cast<T>(1.22173047639603070384658353794);
	constexpr auto ref_w = static_cast<T>(320);
	constexpr auto ref_h = static_cast<T>(180);
	constexpr auto ref_n = static_cast<T>(0.5);
	constexpr auto ref_f = static_cast<T>(25);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(0.80333325379243947),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(1.4281480067421146),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(-1.0408163265306123),
		static_cast<T>(-1),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(-1.0204081632653061),
		static_cast<T>(0),
	};

	constexpr auto epsilon = 1 / static_cast<T>(100'000);
	const auto value = bstone::cgm::make_perspective_vfov_rh_n1p1(ref_v, ref_w, ref_h, ref_n, ref_f);

	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

void test_make_ortho_rh_n1p1_44f()
{
	test_make_ortho_rh_n1p1<4, 4, float>();
}

void test_make_ortho_rh_n1p1_44d()
{
	test_make_ortho_rh_n1p1<4, 4, double>();
}

// ==========================================================================

void test_make_perspective_vfov_rh_n1p1_44f()
{
	make_perspective_vfov_rh_n1p1<4, 4, float>();
}

void test_make_perspective_vfov_rh_n1p1_44d()
{
	make_perspective_vfov_rh_n1p1<4, 4, double>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_make_ortho_rh_n1p1();
		register_make_perspective_vfov_rh_n1p1();
	}

private:
	void register_make_ortho_rh_n1p1()
	{
		tester.register_test("cgm::make_ortho_rh_n1p1#e1l76g5urreorayj", test_make_ortho_rh_n1p1_44f);
		tester.register_test("cgm::make_ortho_rh_n1p1#aq6vga2j0n982ppm", test_make_ortho_rh_n1p1_44d);
	}

	void register_make_perspective_vfov_rh_n1p1()
	{
		tester.register_test("cgm::make_perspective_vfov_rh_n1p1#waz3nf3lnzpyhmcp", test_make_perspective_vfov_rh_n1p1_44f);
		tester.register_test("cgm::make_perspective_vfov_rh_n1p1#y8r8wkfue8wozfwa", test_make_perspective_vfov_rh_n1p1_44d);
	}
};

auto registrator = Registrator{};

} // namespace
