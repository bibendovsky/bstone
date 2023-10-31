#include <cmath>

#include "bstone_tester.h"

#include "bstone_cgm_transform.h"

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
void test_make_translation()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_x = static_cast<T>(10);
	constexpr auto ref_y = static_cast<T>(-20);
	constexpr auto ref_z = static_cast<T>(30);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(1),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(1),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
		static_cast<T>(0),

		static_cast<T>(10),
		static_cast<T>(-20),
		static_cast<T>(30),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);
	constexpr auto value = bstone::cgm::make_translation(Vec3{ref_x, ref_y, ref_z});

	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_translate()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_x1 = static_cast<T>(10);
	constexpr auto ref_y1 = static_cast<T>(-20);
	constexpr auto ref_z1 = static_cast<T>(30);
	constexpr auto ref_x2 = static_cast<T>(-50);
	constexpr auto ref_y2 = static_cast<T>(70);
	constexpr auto ref_z2 = static_cast<T>(40);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(1),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(1),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
		static_cast<T>(0),

		static_cast<T>(-40),
		static_cast<T>(50),
		static_cast<T>(70),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);

	constexpr auto value =
		bstone::cgm::translate(
			bstone::cgm::translate(
				Mat::get_identity(),
				Vec3{ref_x1, ref_y1, ref_z1}),
			Vec3{ref_x2, ref_y2, ref_z2});


	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_make_scale()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_x = static_cast<T>(10);
	constexpr auto ref_y = static_cast<T>(-20);
	constexpr auto ref_z = static_cast<T>(30);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(10),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(-20),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(30),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);
	constexpr auto value = bstone::cgm::make_scale(Vec3{ref_x, ref_y, ref_z});

	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_scale()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_x1 = static_cast<T>(10);
	constexpr auto ref_y1 = static_cast<T>(-20);
	constexpr auto ref_z1 = static_cast<T>(30);
	constexpr auto ref_x2 = static_cast<T>(-50);
	constexpr auto ref_y2 = static_cast<T>(70);
	constexpr auto ref_z2 = static_cast<T>(40);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(-500),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(-1400),
		static_cast<T>(0),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1200),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);

	constexpr auto value =
		bstone::cgm::scale(
			bstone::cgm::scale(
				Mat::get_identity(),
				Vec3{ref_x1, ref_y1, ref_z1}),
			Vec3{ref_x2, ref_y2, ref_z2});


	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_make_rotation()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_a = static_cast<T>(0.174532925199432957692369076849L);
	constexpr auto ref_x = static_cast<T>(0.534522483824848769369106961760L);
	constexpr auto ref_y = static_cast<T>(0.267261241912424384684553480880L);
	constexpr auto ref_z = static_cast<T>(0.801783725737273154053660442639L);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(0.98914837837219238),
		static_cast<T>(0.14139860868453979),
		static_cast<T>(-0.039898455142974854),
		static_cast<T>(0),

		static_cast<T>(-0.13705796003341675),
		static_cast<T>(0.98589289188385010),
		static_cast<T>(0.096074342727661133),
		static_cast<T>(0),

		static_cast<T>(0.052920401096343994),
		static_cast<T>(-0.089563369750976562),
		static_cast<T>(0.99457418918609619),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);
	const auto value = bstone::cgm::make_rotation(ref_a, Vec3{ref_x, ref_y, ref_z});

	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

template<std::intptr_t M, std::intptr_t N, typename T>
void test_rotate()
{
	using Mat = bstone::cgm::Mat<M, N, T>;
	using Vec3 = bstone::cgm::Vec<3, T>;

	constexpr auto ref_a1 = static_cast<T>(0.174532925199432957692369076849);
	constexpr auto ref_x1 = static_cast<T>(0.534522483824848769369106961760);
	constexpr auto ref_y1 = static_cast<T>(0.267261241912424384684553480880);
	constexpr auto ref_z1 = static_cast<T>(0.801783725737273154053660442639);
	constexpr auto ref_a2 = static_cast<T>(0.0872664625997164788461845384244);
	constexpr auto ref_x2 = static_cast<T>(-0.526710471450893642522029196105);
	constexpr auto ref_y2 = static_cast<T>(0.771254618910237119407257037154);
	constexpr auto ref_z2 = static_cast<T>(-0.357410677055963543139948383071);

	constexpr auto ref_value = Mat
	{
		static_cast<T>(0.98739057779312134),
		static_cast<T>(0.11473109573125839),
		static_cast<T>(-0.10907210409641266),
		static_cast<T>(0),

		static_cast<T>(-0.11004818975925446),
		static_cast<T>(0.99276435375213623),
		static_cast<T>(0.048045039176940918),
		static_cast<T>(0),

		static_cast<T>(0.11379514634609222),
		static_cast<T>(-0.035435989499092102),
		static_cast<T>(0.99287199974060059),
		static_cast<T>(0),

		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(0),
		static_cast<T>(1),
	};

	constexpr auto epsilon = static_cast<T>(1) / static_cast<T>(100'000);

	const auto value =
		bstone::cgm::rotate(
			bstone::cgm::rotate(
				Mat::get_identity(),
				ref_a1,
				Vec3{ref_x1, ref_y1, ref_z1}),
			ref_a2,
			Vec3{ref_x2, ref_y2, ref_z2});


	tester.check(are_mat4_equal(value, ref_value, epsilon));
}

// ==========================================================================

void test_make_translation_44f()
{
	test_make_translation<4, 4, float>();
}

void test_make_translation_44d()
{
	test_make_translation<4, 4, double>();
}

// ==========================================================================

void test_translate_44f()
{
	test_translate<4, 4, float>();
}

void test_translate_44d()
{
	test_translate<4, 4, double>();
}

// ==========================================================================

void test_make_scale_44f()
{
	test_make_scale<4, 4, float>();
}

void test_make_scale_44d()
{
	test_make_scale<4, 4, double>();
}

// ==========================================================================

void test_scale_44f()
{
	test_scale<4, 4, float>();
}

void test_scale_44d()
{
	test_scale<4, 4, double>();
}

// ==========================================================================

void test_make_rotation_44f()
{
	test_make_rotation<4, 4, float>();
}

void test_make_rotation_44d()
{
	test_make_rotation<4, 4, double>();
}

// ==========================================================================

void test_rotate_44f()
{
	test_rotate<4, 4, float>();
}

void test_rotate_44d()
{
	test_rotate<4, 4, double>();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_make_translation();
		register_translate();
		register_make_scale();
		register_scale();
		register_make_rotation();
		register_rotate();
	}

private:
	void register_make_translation()
	{
		tester.register_test("cgm::make_translation#xfxji4ht8i2zz9d3", test_make_translation_44f);
		tester.register_test("cgm::make_translation#js1vasjr8nst7jkh", test_make_translation_44d);
	}

	void register_translate()
	{
		tester.register_test("cgm::translate#k4hfoymo8sy6rszy", test_translate_44f);
		tester.register_test("cgm::translate#730rsyqu1jbfisg1", test_translate_44d);
	}

	void register_make_scale()
	{
		tester.register_test("cgm::make_scale#stguk2joucfgps1i", test_make_scale_44f);
		tester.register_test("cgm::make_scale#drc5rwhohh1rs2bn", test_make_scale_44d);
	}

	void register_scale()
	{
		tester.register_test("cgm::scale#k4hfoymo8sy6rszy", test_scale_44f);
		tester.register_test("cgm::scale#730rsyqu1jbfisg1", test_scale_44d);
	}

	void register_make_rotation()
	{
		tester.register_test("cgm::make_rotation#hnmslwjdzv6ckmz9", test_make_rotation_44f);
		tester.register_test("cgm::make_rotation#xzdwxpqlc7nmq4du", test_make_rotation_44d);
	}

	void register_rotate()
	{
		tester.register_test("cgm::rotate#exoi1rrup0zhe7rk", test_rotate_44f);
		tester.register_test("cgm::rotate#t39ayd7779ril8r6", test_rotate_44d);
	}
};

auto registrator = Registrator{};

} // namespace
