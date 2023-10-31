#include "bstone_memory_pool_bitmap.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// is_empty()
// Empty.
void test_fueg7ghcto08d19a()
{
	using Bitmap = bstone::StaticMemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	tester.check(bitmap.is_empty());
}

// is_empty()
// Not empty.
void test_80hunq7s2038ygff()
{
	using Bitmap = bstone::StaticMemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	bitmap.set_first_free();
	tester.check(!bitmap.is_empty());
}

// ==========================================================================

// set_first_free()
// Success.
void test_gm7nai4469mr4mha()
{
	using Bitmap = bstone::StaticMemoryPoolBitmap<2>;
	auto bitmap = Bitmap{};
	const auto index1 = bitmap.set_first_free();
	const auto index2 = bitmap.set_first_free();
	tester.check(index1 == 0 && index2 == 1);
}

// set_first_free()
// Out of memory.
void test_npdaf3n62fwtnnou()
{
	using Bitmap = bstone::StaticMemoryPoolBitmap<2>;
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

// reset(std::intptr_t)
// Success.
void test_79zcovx4jptabjnr()
{
	using Bitmap = bstone::StaticMemoryPoolBitmap<2>;
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

// is_empty()
// Empty.
void test_q9yril6iup92impx()
{
	using Bitmap = bstone::DynamicMemoryPoolBitmap;
	auto bitmap = Bitmap{};
	bitmap.resize(2, bstone::get_default_memory_resource());
	tester.check(bitmap.is_empty());
}

// is_empty()
// Not empty.
void test_qxb66zil9az01j91()
{
	using Bitmap = bstone::DynamicMemoryPoolBitmap;
	auto bitmap = Bitmap{};
	bitmap.resize(2, bstone::get_default_memory_resource());
	bitmap.set_first_free();
	tester.check(!bitmap.is_empty());
}

// ==========================================================================

// set_first_free()
// Success.
void test_c6l099z6wg21372i()
{
	using Bitmap = bstone::DynamicMemoryPoolBitmap;
	auto bitmap = Bitmap{};
	bitmap.resize(2, bstone::get_default_memory_resource());
	const auto index1 = bitmap.set_first_free();
	const auto index2 = bitmap.set_first_free();
	tester.check(index1 == 0 && index2 == 1);
}

// set_first_free()
// Out of memory.
void test_dgjpr3kc2id0b23x()
{
	using Bitmap = bstone::DynamicMemoryPoolBitmap;
	auto bitmap = Bitmap{};
	bitmap.resize(2, bstone::get_default_memory_resource());
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

// reset(std::intptr_t)
// Success.
void test_ldm3uuyunyf2f2lc()
{
	using Bitmap = bstone::DynamicMemoryPoolBitmap;
	auto bitmap = Bitmap{};
	bitmap.resize(2, bstone::get_default_memory_resource());
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
		register_static_memory_pool_bitmap();
		register_dynamic_memory_pool_bitmap();
	}

private:
	void register_static_memory_pool_bitmap()
	{
		tester.register_test("StaticMemoryPoolBitmap#fueg7ghcto08d19a", test_fueg7ghcto08d19a);
		tester.register_test("StaticMemoryPoolBitmap#80hunq7s2038ygff", test_80hunq7s2038ygff);
		tester.register_test("StaticMemoryPoolBitmap#gm7nai4469mr4mha", test_gm7nai4469mr4mha);
		tester.register_test("StaticMemoryPoolBitmap#npdaf3n62fwtnnou", test_npdaf3n62fwtnnou);
		tester.register_test("StaticMemoryPoolBitmap#79zcovx4jptabjnr", test_79zcovx4jptabjnr);
	}

	void register_dynamic_memory_pool_bitmap()
	{
		tester.register_test("DynamicMemoryPoolBitmap#q9yril6iup92impx", test_q9yril6iup92impx);
		tester.register_test("DynamicMemoryPoolBitmap#qxb66zil9az01j91", test_qxb66zil9az01j91);
		tester.register_test("DynamicMemoryPoolBitmap#c6l099z6wg21372i", test_c6l099z6wg21372i);
		tester.register_test("DynamicMemoryPoolBitmap#dgjpr3kc2id0b23x", test_dgjpr3kc2id0b23x);
		tester.register_test("DynamicMemoryPoolBitmap#ldm3uuyunyf2f2lc", test_ldm3uuyunyf2f2lc);
	}
};

auto registrator = Registrator{};

} // namespace
