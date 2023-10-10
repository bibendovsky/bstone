#include "bstone_tester.h"

#include "bstone_int.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

template<typename TSigned, typename TUnsigned, long long TMaxReference>
void test_intx_generic()
{
	static_assert(sizeof(TSigned) == sizeof(TUnsigned), "Invalid input types.");

	// Maximum positive value.
	auto max_value_u = TUnsigned{}; // Clear all bits.
	--max_value_u; // Set all bits.
	max_value_u >>= 1; // Clear most significand bit.
	const auto max_value = static_cast<TSigned>(max_value_u); // Cast to signed.

	// Minimum negative value.
	auto min_value_u = max_value_u; // Set all bits.
	++min_value_u; // Clear all bits but most significand.
	const auto min_value = static_cast<TSigned>(min_value_u); // Cast to signed.

	tester.check(min_value == (-TMaxReference - 1) && max_value == TMaxReference);
}

template<typename T, unsigned long long TRef>
void test_uintx_generic()
{
	auto max_value = T{}; // Clear all bits.
	--max_value; // Set all bits.

	tester.check(max_value == TRef);
}

// --------------------------------------------------------------------------

template<std::size_t TSize>
class TestIntP
{
public:
	void operator()() const
	{
		tester.fail("Unknown platform.");
	}
};

template<>
class TestIntP<4>
{
public:
	void operator()() const
	{
		test_intx_generic<bstone::IntP, bstone::UIntP, 2'147'483'647LL>();
	}
};

template<>
class TestIntP<8>
{
public:
	void operator()() const
	{
		test_intx_generic<bstone::IntP, bstone::UIntP, 9'223'372'036'854'775'807LL>();
	}
};

// --------------------------------------------------------------------------

template<std::size_t TSize>
class TestUIntP
{
public:
	void operator()() const
	{
		tester.fail("Unknown platform.");
	}
};

template<>
class TestUIntP<4>
{
public:
	void operator()() const
	{
		test_uintx_generic<bstone::UIntP, 4'294'967'295ULL>();
	}
};

template<>
class TestUIntP<8>
{
public:
	void operator()() const
	{
		test_uintx_generic<bstone::UIntP, 18'446'744'073'709'551'615ULL>();
	}
};

// --------------------------------------------------------------------------

void test_de9dg7kir86o0uu7()
{
	test_intx_generic<bstone::Int8, bstone::UInt8, 127LL>();
}

void test_18xkw98spdyanng8()
{
	test_uintx_generic<bstone::UInt8, 255ULL>();
}

void test_vgyvun5abio0jv3z()
{
	test_intx_generic<bstone::Int16, bstone::UInt16, 32'767LL>();
}

void test_lmi9eqjzsortihch()
{
	test_uintx_generic<bstone::UInt16, 65'535ULL>();
}

void test_66itosweyhvnejon()
{
	test_intx_generic<bstone::Int32, bstone::UInt32, 2'147'483'647LL>();
}

void test_zpwsnojs9vc07lnt()
{
	test_uintx_generic<bstone::UInt32, 4'294'967'295ULL>();
}

void test_dv986i0khrohkbkk()
{
	test_intx_generic<bstone::Int64, bstone::UInt64, 9'223'372'036'854'775'807LL>();
}

void test_e9yfawly3ky1qb7z()
{
	test_uintx_generic<bstone::UInt64, 18'446'744'073'709'551'615ULL>();
}

void test_yewvcabpxykd7g3w()
{
	TestIntP<sizeof(bstone::IntP)>{}();
}

void test_1hoinm18t7nxc1da()
{
	TestUIntP<sizeof(bstone::UIntP)>{}();
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		tester.register_test("Int8", test_de9dg7kir86o0uu7);
		tester.register_test("UInt8", test_18xkw98spdyanng8);
		tester.register_test("Int16", test_vgyvun5abio0jv3z);
		tester.register_test("UInt16", test_lmi9eqjzsortihch);
		tester.register_test("Int32", test_66itosweyhvnejon);
		tester.register_test("UInt32", test_zpwsnojs9vc07lnt);
		tester.register_test("Int64", test_dv986i0khrohkbkk);
		tester.register_test("UInt64", test_e9yfawly3ky1qb7z);
		tester.register_test("IntP", test_yewvcabpxykd7g3w);
		tester.register_test("UIntP", test_1hoinm18t7nxc1da);
	}
};

const auto registrator = Registrator{};

} // namespace
