#include <cstddef>

#include "bstone_ts_auto_arena_resource.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// TsAutoArenaResource(std::intptr_t, MemoryResource&)
void test_5p6cz74sww15h0uk()
{
	bstone::TsAutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
}

// ==========================================================================

// std::intptr_t get_capacity() noexcept
void test_tdpwy14xt5fneiaq()
{
	bstone::TsAutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
	const auto is_valid_1 = memory_resource.get_capacity() == 10;

	const auto is_valid =
		is_valid_1 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// std::intptr_t get_size() noexcept
void test_jh2gg2389ndixbvw()
{
	bstone::TsAutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
	const auto is_valid_1 = memory_resource.get_size() == 0;

	static_cast<void>(memory_resource.allocate(5));
	const auto is_valid_2 = memory_resource.get_size() == 5;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// reserve(std::intptr_t, MemoryResource&)
// Success.
void test_3oatdmwi1xqy0y10()
{
	using MemoryResource = bstone::TsAutoArenaResource;

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

// allocate(std::intptr_t) and deallocate(void*)
// Success.
void test_lhm3t09kkosrv0pf()
{
	using MemoryResource = bstone::TsAutoArenaResource;

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
	using MemoryResource = bstone::TsAutoArenaResource;

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
// Out of memory.
void test_hj5io0671lpsiro3()
{
	using MemoryResource = bstone::TsAutoArenaResource;

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
		register_ctor();
		register_get_capacity();
		register_get_size();
		register_ts_auto_arena_memory_resource();
	}

private:
	void register_ctor()
	{
		tester.register_test("TsAutoArenaResource#5p6cz74sww15h0uk", test_5p6cz74sww15h0uk);
	}

	void register_get_capacity()
	{
		tester.register_test("TsAutoArenaResource#tdpwy14xt5fneiaq", test_tdpwy14xt5fneiaq);
	}

	void register_get_size()
	{
		tester.register_test("TsAutoArenaResource#jh2gg2389ndixbvw", test_jh2gg2389ndixbvw);
	}

	void register_ts_auto_arena_memory_resource()
	{
		tester.register_test("TsAutoArenaResource#3oatdmwi1xqy0y10", test_3oatdmwi1xqy0y10);
		tester.register_test("TsAutoArenaResource#lhm3t09kkosrv0pf", test_lhm3t09kkosrv0pf);
		tester.register_test("TsAutoArenaResource#h3zddrzsqc41lln6", test_h3zddrzsqc41lln6);
		tester.register_test("TsAutoArenaResource#hj5io0671lpsiro3", test_hj5io0671lpsiro3);
	}
};

auto registrator = Registrator{};

} // namespace
