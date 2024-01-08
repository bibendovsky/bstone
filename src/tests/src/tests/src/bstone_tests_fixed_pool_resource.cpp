#include "bstone_tester.h"

#include "bstone_fixed_pool_resource.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// allocate(std::size_t)
// Default.
void test_nrkmtbnuex9y6k5e()
{
	using Item = double;
	using MemoryPool = bstone::FixedPoolResource<Item, 1>;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		auto ptr = memory_pool.allocate(sizeof(Item));
		memory_pool.deallocate(ptr);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed);
}

// allocate(std::size_t)
// Size mismatch.
void test_9v82vg27zs4781w5()
{
	using Item = double;
	using MemoryPool = bstone::FixedPoolResource<Item, 1>;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		auto ptr = memory_pool.allocate(0);
		memory_pool.deallocate(ptr);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// allocate(std::size_t)
// Out of memory.
void test_uyubiuemn20zxum3()
{
	using Item = double;
	using MemoryPool = bstone::FixedPoolResource<Item, 1>;

	auto is_failed = false;

	void* ptrs[2] = {};

	MemoryPool memory_pool{};

	try
	{
		ptrs[0] = memory_pool.allocate(sizeof(Item));
		ptrs[1] = memory_pool.allocate(sizeof(Item));
	}
	catch (...)
	{
		is_failed = true;
	}

	memory_pool.deallocate(ptrs[1]);
	memory_pool.deallocate(ptrs[0]);

	tester.check(is_failed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_fixed_memory_pool_allocate();
	}

private:
	void register_fixed_memory_pool_allocate()
	{
		tester.register_test("FixedPoolResource#nrkmtbnuex9y6k5e", test_nrkmtbnuex9y6k5e);
		tester.register_test("FixedPoolResource#9v82vg27zs4781w5", test_9v82vg27zs4781w5);
		tester.register_test("FixedPoolResource#uyubiuemn20zxum3", test_uyubiuemn20zxum3);
	}
};

auto registrator = Registrator{};

} // namespace
