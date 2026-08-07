#include "bstone_r3r_sample_count.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// unsigned int clamp_bitmask(unsigned int)
// Keeps every sample count up to the limit.
void test_z2j4qk8fxr0m5vwd()
{
	constexpr auto bitmask = bstone::R3rSampleCount::clamp_bitmask(0x3FU);
	const auto is_valid_value = bitmask == 0x3FU;
	tester.check(is_valid_value);
}

// unsigned int clamp_bitmask(unsigned int)
// Keeps the limit itself.
void test_p7c1nubs9lta3ke6()
{
	constexpr auto bitmask = bstone::R3rSampleCount::clamp_bitmask(
		static_cast<unsigned int>(bstone::R3rLimits::max_aa));
	const auto is_valid_value = bitmask == static_cast<unsigned int>(bstone::R3rLimits::max_aa);
	tester.check(is_valid_value);
}

// unsigned int clamp_bitmask(unsigned int)
// Drops the sample counts beyond the limit.
void test_h5v8drqy1o6wgn3t()
{
	constexpr auto bitmask = bstone::R3rSampleCount::clamp_bitmask(0xFFU);
	const auto is_valid_value = bitmask == 0x3FU;
	tester.check(is_valid_value);
}

// unsigned int clamp_bitmask(unsigned int)
// An empty mask stays empty.
void test_a0mzl6ei4rf2sxq9()
{
	constexpr auto bitmask = bstone::R3rSampleCount::clamp_bitmask(0U);
	const auto is_valid_value = bitmask == 0U;
	tester.check(is_valid_value);
}

// ==========================================================================

// int choose(unsigned int, int)
// The degree is an upper bound.
void test_k3wq7t5ybn0icfa8()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0x3FU, 8);
	const auto is_valid_value = sample_count == 8;
	tester.check(is_valid_value);
}

// int choose(unsigned int, int)
// The greatest sample count of the mask not exceeding the degree.
void test_q9dxu2ml7gvopr41()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0x05U, 8);
	const auto is_valid_value = sample_count == 4;
	tester.check(is_valid_value);
}

// int choose(unsigned int, int)
// The limit itself is choosable.
void test_t6bnf3zwc8shje2p()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0x3FU, bstone::R3rLimits::max_aa);
	const auto is_valid_value = sample_count == bstone::R3rLimits::max_aa;
	tester.check(is_valid_value);
}

// int choose(unsigned int, int)
// A degree beyond the limit doesn't exceed it.
void test_x1grso4kdv9ylb7m()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0xFFU, 1024);
	const auto is_valid_value = sample_count == bstone::R3rLimits::max_aa;
	tester.check(is_valid_value);
}

// int choose(unsigned int, int)
// No anti-aliasing when nothing fits the degree.
void test_v4ehp0nq6yui8w3c()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0x20U, 16);
	const auto is_valid_value = sample_count == 1;
	tester.check(is_valid_value);
}

// int choose(unsigned int, int)
// No anti-aliasing for an empty mask.
void test_m2sfk9rjxo5taz1u()
{
	constexpr auto sample_count = bstone::R3rSampleCount::choose(0U, bstone::R3rLimits::max_aa);
	const auto is_valid_value = sample_count == 1;
	tester.check(is_valid_value);
}

// ==========================================================================

// int get_max(unsigned int)
// The greatest sample count of the mask.
void test_r8ylc3wgn1vke67d()
{
	constexpr auto sample_count = bstone::R3rSampleCount::get_max(0x15U);
	const auto is_valid_value = sample_count == 16;
	tester.check(is_valid_value);
}

// int get_max(unsigned int)
// The limit is reachable.
void test_j5ot2xbma9qz4hpw()
{
	constexpr auto sample_count = bstone::R3rSampleCount::get_max(0x7FU);
	const auto is_valid_value = sample_count == bstone::R3rLimits::max_aa;
	tester.check(is_valid_value);
}

// int get_max(unsigned int)
// No anti-aliasing for an empty mask.
void test_w0izq6dtn3ub8fys()
{
	constexpr auto sample_count = bstone::R3rSampleCount::get_max(0U);
	const auto is_valid_value = sample_count == 1;
	tester.check(is_valid_value);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_clamp_bitmask();
		register_choose();
		register_get_max();
	}

private:
	void register_clamp_bitmask()
	{
		tester.register_test("R3rSampleCount#z2j4qk8fxr0m5vwd", test_z2j4qk8fxr0m5vwd);
		tester.register_test("R3rSampleCount#p7c1nubs9lta3ke6", test_p7c1nubs9lta3ke6);
		tester.register_test("R3rSampleCount#h5v8drqy1o6wgn3t", test_h5v8drqy1o6wgn3t);
		tester.register_test("R3rSampleCount#a0mzl6ei4rf2sxq9", test_a0mzl6ei4rf2sxq9);
	}

	void register_choose()
	{
		tester.register_test("R3rSampleCount#k3wq7t5ybn0icfa8", test_k3wq7t5ybn0icfa8);
		tester.register_test("R3rSampleCount#q9dxu2ml7gvopr41", test_q9dxu2ml7gvopr41);
		tester.register_test("R3rSampleCount#t6bnf3zwc8shje2p", test_t6bnf3zwc8shje2p);
		tester.register_test("R3rSampleCount#x1grso4kdv9ylb7m", test_x1grso4kdv9ylb7m);
		tester.register_test("R3rSampleCount#v4ehp0nq6yui8w3c", test_v4ehp0nq6yui8w3c);
		tester.register_test("R3rSampleCount#m2sfk9rjxo5taz1u", test_m2sfk9rjxo5taz1u);
	}

	void register_get_max()
	{
		tester.register_test("R3rSampleCount#r8ylc3wgn1vke67d", test_r8ylc3wgn1vke67d);
		tester.register_test("R3rSampleCount#j5ot2xbma9qz4hpw", test_j5ot2xbma9qz4hpw);
		tester.register_test("R3rSampleCount#w0izq6dtn3ub8fys", test_w0izq6dtn3ub8fys);
	}
};

auto registrator = Registrator{};

} // namespace
