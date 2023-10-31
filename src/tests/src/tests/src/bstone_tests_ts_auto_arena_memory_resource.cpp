#include <cstddef>

#include "bstone_ts_auto_arena_memory_resource.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// reserve(std::intptr_t, MemoryResource&)
// Success.
void test_3oatdmwi1xqy0y10()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;

	try
	{
		MemoryResource memory_resource{};
		memory_resource.reserve(10, bstone::get_default_memory_resource());
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed);
}

// reserve(std::intptr_t, MemoryResource&)
// Negative size.
void test_o5jnbo3ulgrdbsmf()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;

	try
	{
		MemoryResource memory_resource{};
		memory_resource.reserve(-1, bstone::get_default_memory_resource());
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// allocate(std::intptr_t) and deallocate(void*)
// Success.
void test_lhm3t09kkosrv0pf()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;
	auto diff = std::ptrdiff_t{};

	try
	{
		MemoryResource memory_resource{};
		memory_resource.reserve(10, bstone::get_default_memory_resource());
		const auto ptr1 = static_cast<unsigned char*>(memory_resource.allocate(5));
		const auto ptr2 = static_cast<unsigned char*>(memory_resource.allocate(5));
		diff = ptr2 - ptr1;
		memory_resource.deallocate(ptr2);
		memory_resource.deallocate(ptr1);
		const auto ptr3 = memory_resource.allocate(5);
		memory_resource.deallocate(ptr3);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && diff == 5);
}

// allocate(std::intptr_t) and deallocate(void*)
// Zero size.
void test_h3zddrzsqc41lln6()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;
	auto diff = std::ptrdiff_t{};

	try
	{
		MemoryResource memory_resource{};
		memory_resource.reserve(10, bstone::get_default_memory_resource());
		const auto ptr1 = static_cast<unsigned char*>(memory_resource.allocate(0));
		const auto ptr2 = static_cast<unsigned char*>(memory_resource.allocate(0));
		diff = ptr2 - ptr1;
		memory_resource.deallocate(ptr2);
		memory_resource.deallocate(ptr1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && diff == 1);
}

// allocate(std::intptr_t)
// Negative size.
void test_4pxq7pc8gjg5dw41()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;

	try
	{
		MemoryResource memory_resource{};
		memory_resource.reserve(10, bstone::get_default_memory_resource());
		memory_resource.allocate(-1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// allocate(std::intptr_t)
// Out of memory.
void test_hj5io0671lpsiro3()
{
	using MemoryResource = bstone::TsAutoArenaMemoryResource;

	auto is_failed = false;

	try
	{
		MemoryResource memory_resource{};
		memory_resource.allocate(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ts_auto_arena_memory_resource();
	}

private:
	void register_ts_auto_arena_memory_resource()
	{
		tester.register_test("TsAutoArenaMemoryResource#3oatdmwi1xqy0y10", test_3oatdmwi1xqy0y10);
		tester.register_test("TsAutoArenaMemoryResource#o5jnbo3ulgrdbsmf", test_o5jnbo3ulgrdbsmf);
		tester.register_test("TsAutoArenaMemoryResource#lhm3t09kkosrv0pf", test_lhm3t09kkosrv0pf);
		tester.register_test("TsAutoArenaMemoryResource#h3zddrzsqc41lln6", test_h3zddrzsqc41lln6);
		tester.register_test("TsAutoArenaMemoryResource#4pxq7pc8gjg5dw41", test_4pxq7pc8gjg5dw41);
		tester.register_test("TsAutoArenaMemoryResource#hj5io0671lpsiro3", test_hj5io0671lpsiro3);
	}
};

auto registrator = Registrator{};

} // namespace
