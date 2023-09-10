#include "bstone_memory_pool_bitmap.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// is_empty()
// Empty.
void test_fueg7ghcto08d19a()
{
	using Bitmap = bstone::MemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	tester.check(bitmap.is_empty());
}

// is_empty()
// Not empty.
void test_80hunq7s2038ygff()
{
	using Bitmap = bstone::MemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	bitmap.set_first_free();
	tester.check(!bitmap.is_empty());
}

// ==========================================================================

// set_first_free()
// Success.
void test_gm7nai4469mr4mha()
{
	using Bitmap = bstone::MemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	const auto index1 = bitmap.set_first_free();
	const auto index2 = bitmap.set_first_free();
	tester.check(index1 == 0 && index2 == 1);
}

// set_first_free()
// Out of memory.
void test_npdaf3n62fwtnnou()
{
	using Bitmap = bstone::MemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	const auto index1 = bitmap.set_first_free();
	const auto index2 = bitmap.set_first_free();

	auto is_failed = false;

	try
	{
		bitmap.set_first_free();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(index1 == 0 && index2 == 1 && is_failed);
}

// ==========================================================================

// reset(MemoryPoolBitmapInt)
// Success.
void test_79zcovx4jptabjnr()
{
	using Bitmap = bstone::MemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	const auto index1 = bitmap.set_first_free();
	const auto index2 = bitmap.set_first_free();
	const auto is_non_empty = !bitmap.is_empty();
	bitmap.reset(index1);
	bitmap.reset(index2);
	const auto is_empty = bitmap.is_empty();
	tester.check(index1 == 0 && index2 == 1 && is_non_empty && is_empty);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_memory_pool_bitmap();
	}

private:
	void register_memory_pool_bitmap()
	{
		tester.register_test("MemoryPoolBitmap#fueg7ghcto08d19a", test_fueg7ghcto08d19a);
		tester.register_test("MemoryPoolBitmap#80hunq7s2038ygff", test_80hunq7s2038ygff);
		tester.register_test("MemoryPoolBitmap#gm7nai4469mr4mha", test_gm7nai4469mr4mha);
		tester.register_test("MemoryPoolBitmap#npdaf3n62fwtnnou", test_npdaf3n62fwtnnou);
		tester.register_test("MemoryPoolBitmap#79zcovx4jptabjnr", test_79zcovx4jptabjnr);
	}
};

auto registrator = Registrator{};

} // namespace
