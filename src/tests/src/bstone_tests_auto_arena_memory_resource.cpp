#include <cstddef>

#include "bstone_auto_arena_memory_resource.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// reserve(IntP, MemoryResource&)
// Success.
void test_12i9vgaja0au5hw4()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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

// reserve(IntP, MemoryResource&)
// Negative size.
void test_4hi9u7tj6ugr8kmy()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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

// allocate(IntP) and deallocate(void*)
// Success.
void test_h89q7m1ofox5m1yt()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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

// allocate(IntP) and deallocate(void*)
// Zero size.
void test_7toaug4pnm1eyjyx()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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

// allocate(IntP)
// Negative size.
void test_cz3bzq9ia1144mgd()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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

// allocate(IntP)
// Out of memory.
void test_16hxh0gkqffhccg1()
{
	using MemoryResource = bstone::AutoArenaMemoryResource;

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
		register_auto_arena_memory_resource();
	}

private:
	void register_auto_arena_memory_resource()
	{
		tester.register_test("AutoArenaMemoryResource#12i9vgaja0au5hw4", test_12i9vgaja0au5hw4);
		tester.register_test("AutoArenaMemoryResource#4hi9u7tj6ugr8kmy", test_4hi9u7tj6ugr8kmy);
		tester.register_test("AutoArenaMemoryResource#h89q7m1ofox5m1yt", test_h89q7m1ofox5m1yt);
		tester.register_test("AutoArenaMemoryResource#7toaug4pnm1eyjyx", test_7toaug4pnm1eyjyx);
		tester.register_test("AutoArenaMemoryResource#cz3bzq9ia1144mgd", test_cz3bzq9ia1144mgd);
		tester.register_test("AutoArenaMemoryResource#16hxh0gkqffhccg1", test_16hxh0gkqffhccg1);
	}
};

auto registrator = Registrator{};

} // namespace
