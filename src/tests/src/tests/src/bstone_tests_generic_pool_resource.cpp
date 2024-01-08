#include "bstone_tester.h"

#include "bstone_generic_pool_resource.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// allocate(std::size_t)
// Default.
void test_4gaip46ok4ygv2x2()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		memory_pool.reserve(ItemSize, 1, bstone::get_default_memory_resource());
		auto ptr = memory_pool.allocate(static_cast<std::size_t>(ItemSize));
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
void test_5qgvnqpxgycv7gkd()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		memory_pool.reserve(ItemSize, 1, bstone::get_default_memory_resource());
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
void test_gam4600nv3dkpjuq()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	void* ptrs[2] = {};

	MemoryPool memory_pool{};
	memory_pool.reserve(ItemSize, 1, bstone::get_default_memory_resource());

	try
	{
		ptrs[0] = memory_pool.allocate(ItemSize);
		ptrs[1] = memory_pool.allocate(ItemSize);
	}
	catch (...)
	{
		is_failed = true;
	}

	memory_pool.deallocate(ptrs[1]);
	memory_pool.deallocate(ptrs[0]);

	tester.check(is_failed);
}

// --------------------------------------------------------------------------

class CustomMemoryResource final : public bstone::MemoryResource
{
public:
	CustomMemoryResource() = default;
	~CustomMemoryResource() override = default;

private:
	void* do_allocate(std::intptr_t size) override
	{
		return ::operator new(size);
	}

	void do_deallocate(void* ptr) noexcept override
	{
		::operator delete(ptr);
	}
};

bstone::MemoryResource& get_memory_resource()
{
	static CustomMemoryResource memory_resource{};
	return memory_resource;
}

// allocate(std::size_t)
// Default with custom memory resource.
void test_emcqn7mpnndwaaru()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		memory_pool.reserve(ItemSize, 1, get_memory_resource());
		auto ptr = memory_pool.allocate(static_cast<std::size_t>(ItemSize));
		memory_pool.deallocate(ptr);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed);
}

// allocate(std::size_t)
// Size mismatch with custom memory resource.
void test_u3igyuszzqpfykzv()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	try
	{
		MemoryPool memory_pool{};
		memory_pool.reserve(ItemSize, 1, get_memory_resource());
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
// Out of memory with custom memory resource.
void test_y4t70jtj7dyrvmkt()
{
	constexpr auto ItemSize = static_cast<std::intptr_t>(sizeof(double));
	using MemoryPool = bstone::GenericPoolResource;

	auto is_failed = false;

	void* ptrs[2] = {};

	MemoryPool memory_pool{};
	memory_pool.reserve(ItemSize, 1, get_memory_resource());

	try
	{
		ptrs[0] = memory_pool.allocate(ItemSize);
		ptrs[1] = memory_pool.allocate(ItemSize);
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
		tester.register_test("GenericPoolResource#4gaip46ok4ygv2x2", test_4gaip46ok4ygv2x2);
		tester.register_test("GenericPoolResource#5qgvnqpxgycv7gkd", test_5qgvnqpxgycv7gkd);
		tester.register_test("GenericPoolResource#gam4600nv3dkpjuq", test_gam4600nv3dkpjuq);
		tester.register_test("GenericPoolResource#emcqn7mpnndwaaru", test_emcqn7mpnndwaaru);
		tester.register_test("GenericPoolResource#u3igyuszzqpfykzv", test_u3igyuszzqpfykzv);
		tester.register_test("GenericPoolResource#y4t70jtj7dyrvmkt", test_y4t70jtj7dyrvmkt);
	}
};

auto registrator = Registrator{};

} // namespace
