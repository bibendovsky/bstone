#include "bstone_tester.h"

#include "bstone_single_pool_resource.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// allocate(std::size_t)
// Default.
void test_zn9q5pq1nie16cwt()
{
	using Item = double;
	using MemoryPool = bstone::SinglePoolResource<Item>;

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
void test_2wqkf225zxlfnioc()
{
	using Item = double;
	using MemoryPool = bstone::SinglePoolResource<Item>;

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
void test_kkhgn1orbr9735nl()
{
	using Item = double;
	using MemoryPool = bstone::SinglePoolResource<Item>;

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
		register_single_memory_pool_allocate();
	}

private:
	void register_single_memory_pool_allocate()
	{
		tester.register_test("SinglePoolResource#zn9q5pq1nie16cwt", test_zn9q5pq1nie16cwt);
		tester.register_test("SinglePoolResource#2wqkf225zxlfnioc", test_2wqkf225zxlfnioc);
		tester.register_test("SinglePoolResource#kkhgn1orbr9735nl", test_kkhgn1orbr9735nl);
	}
};

auto registrator = Registrator{};

} // namespace
