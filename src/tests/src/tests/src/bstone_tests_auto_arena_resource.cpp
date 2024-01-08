#include <cstddef>

#include "bstone_auto_arena_resource.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// AutoArenaResource(std::intptr_t, MemoryResource&)
void test_2nuufbimso59d9eh()
{
	const bstone::AutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
}

// ==========================================================================

// std::intptr_t get_capacity() const noexcept
void test_3sitwuhphbjzbed5()
{
	const bstone::AutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
	const auto is_valid_1 = memory_resource.get_capacity() == 10;
	const auto is_valid =
		is_valid_1 &&
		true;
	tester.check(is_valid);
}

// ==========================================================================

// std::intptr_t get_size() const noexcept
void test_wwpbx8htuvik3832()
{
	bstone::AutoArenaResource memory_resource{10, bstone::get_default_memory_resource()};
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
void test_12i9vgaja0au5hw4()
{
	using MemoryResource = bstone::AutoArenaResource;

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
void test_h89q7m1ofox5m1yt()
{
	using MemoryResource = bstone::AutoArenaResource;

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
void test_7toaug4pnm1eyjyx()
{
	using MemoryResource = bstone::AutoArenaResource;

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
void test_16hxh0gkqffhccg1()
{
	using MemoryResource = bstone::AutoArenaResource;

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
		register_auto_arena_memory_resource();
	}

private:
	void register_ctor()
	{
		tester.register_test("AutoArenaResource#2nuufbimso59d9eh", test_2nuufbimso59d9eh);
	}

	void register_get_capacity()
	{
		tester.register_test("AutoArenaResource#3sitwuhphbjzbed5", test_3sitwuhphbjzbed5);
	}

	void register_get_size()
	{
		tester.register_test("AutoArenaResource#wwpbx8htuvik3832", test_wwpbx8htuvik3832);
	}

	void register_auto_arena_memory_resource()
	{
		tester.register_test("AutoArenaResource#12i9vgaja0au5hw4", test_12i9vgaja0au5hw4);
		tester.register_test("AutoArenaResource#h89q7m1ofox5m1yt", test_h89q7m1ofox5m1yt);
		tester.register_test("AutoArenaResource#7toaug4pnm1eyjyx", test_7toaug4pnm1eyjyx);
		tester.register_test("AutoArenaResource#16hxh0gkqffhccg1", test_16hxh0gkqffhccg1);
	}
};

auto registrator = Registrator{};

} // namespace
