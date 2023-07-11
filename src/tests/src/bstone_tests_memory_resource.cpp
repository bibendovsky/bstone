#include "bstone_tester.h"

#include "bstone_memory_resource.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

void test_g6rkthvhbwtdcko3()
{
	auto is_succeeded = false;

	try
	{
		auto memory_resource = bstone::NewDeleteMemoryResource{};
		const auto ptr = memory_resource.allocate(1);
		memory_resource.deallocate(ptr);
		is_succeeded = true;
	}
	catch (const std::exception&) {}

	tester.check(is_succeeded);
}

void test_yvspu8cqcffe2589()
{
	auto is_succeeded = false;

	try
	{
		auto& memory_resource = bstone::get_default_memory_resource();
		const auto ptr = memory_resource.allocate(1);
		memory_resource.deallocate(ptr);
		is_succeeded = true;
	}
	catch (const std::exception&) {}

	tester.check(is_succeeded);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_new_delete();
		register_get_default_memory_resource();
	}

private:
	void register_new_delete()
	{
		tester.register_test("NewDeleteMemoryResource#g6rkthvhbwtdcko3", test_g6rkthvhbwtdcko3);
	}

	void register_get_default_memory_resource()
	{
		tester.register_test("get_default_memory_resource#yvspu8cqcffe2589", test_yvspu8cqcffe2589);
	}
};

auto registrator = Registrator{};

} // namespace
