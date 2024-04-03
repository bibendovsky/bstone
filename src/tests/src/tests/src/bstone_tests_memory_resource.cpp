#include <cstdint>

#include <algorithm>
#include <memory>

#include "bstone_memory_resource.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto memory_fill_value = 'H';

class TestMemoryResource final : public bstone::MemoryResource
{
public:
	TestMemoryResource() = default;
	~TestMemoryResource() override = default;

private:
	BSTONE_CXX_NODISCARD void* do_allocate(std::intptr_t size) override
	{
		const auto ptr = static_cast<std::uint8_t*>(bstone::get_new_delete_memory_resource().allocate(size));
		std::fill_n(ptr, size, memory_fill_value);
		return ptr;
	}

	void do_deallocate(void* ptr) noexcept override
	{
		bstone::get_new_delete_memory_resource().deallocate(ptr);
	}
};

TestMemoryResource test_memory_resource{};

// ==========================================================================

// [[nodiscard]] void* allocate(std::intptr_t)
void test_uvt25w8mr5zio2zq()
{
	const auto ptr = static_cast<char*>(test_memory_resource.allocate(4));

	const auto is_valid =
		ptr[0] == memory_fill_value &&
		ptr[1] == memory_fill_value &&
		ptr[2] == memory_fill_value &&
		ptr[3] == memory_fill_value;

	test_memory_resource.deallocate(ptr);
	tester.check(is_valid);
}

// template<typename T>
// [[nodiscard]] T* allocate(std::intptr_t)
void test_iz71un1jpj7oms2u()
{
	const auto chars32 = test_memory_resource.allocate<char32_t>(2);
	const auto chars = reinterpret_cast<const char*>(chars32);

	const auto is_valid =
		chars[0] == memory_fill_value &&
		chars[1] == memory_fill_value &&
		chars[2] == memory_fill_value &&
		chars[3] == memory_fill_value &&
		chars[4] == memory_fill_value &&
		chars[5] == memory_fill_value &&
		chars[6] == memory_fill_value &&
		chars[7] == memory_fill_value;

	test_memory_resource.deallocate(chars32);
	tester.check(is_valid);
}

// ==========================================================================

// make_memory_resource_uptr(MemoryResource&, TArgs&& ...)
void test_glnf5xdb4bsrfzxd()
{
	class Value
	{
	public:
		Value(int value, bool& is_destructed)
			:
			value_{value},
			is_destructed_{is_destructed}
		{}

		~Value()
		{
			is_destructed_ = true;
		}

		int get_value() const noexcept
		{
			return value_;
		}

	private:
		int value_{};
		bool& is_destructed_;
	};

	constexpr auto ref_value = 100;

	auto is_failed = false;
	auto is_destructed = false;
	auto value = 0;

	try
	{
		const auto value_uptr = bstone::make_memory_resource_uptr<Value>(
			bstone::get_new_delete_memory_resource(),
			ref_value,
			is_destructed);

		value = (*value_uptr).get_value();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_destructed && value == ref_value);
}

// ==========================================================================

// NullMemoryResource and get_null_memory_resource()
void test_yzz2zm9bqm3j4xvs()
{
	auto& memory_resource = bstone::get_null_memory_resource();

	void* ptr = nullptr;
	auto is_allocated = false;

	try
	{
		ptr = memory_resource.allocate(0);
		is_allocated = true;
	}
	catch (...) {}

	auto is_deallocated = false;

	try
	{
		memory_resource.deallocate(ptr);
		is_deallocated = true;
	}
	catch (...) {}

	tester.check(!is_allocated && is_deallocated);
}

// ==========================================================================

// NewDeleteMemoryResource and get_new_delete_memory_resource()
void test_g6rkthvhbwtdcko3()
{
	auto is_succeeded = false;

	try
	{
		auto& memory_resource = bstone::get_new_delete_memory_resource();
		const auto ptr = memory_resource.allocate(1);
		memory_resource.deallocate(ptr);
		is_succeeded = true;
	}
	catch (const std::exception&) {}

	tester.check(is_succeeded);
}

// get_default_memory_resource()
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
		register_allocate();
		register_make_unique_ptr();
		register_null_memory_resource();
		register_new_delete_memory_resource();
		register_get_default_memory_resource();
	}

private:
	void register_allocate()
	{
		tester.register_test("MemoryResource#uvt25w8mr5zio2zq", test_uvt25w8mr5zio2zq);
		tester.register_test("MemoryResource#iz71un1jpj7oms2u", test_iz71un1jpj7oms2u);
	}

	void register_make_unique_ptr()
	{
		tester.register_test("make_memory_resource_uptr#glnf5xdb4bsrfzxd", test_glnf5xdb4bsrfzxd);
	}

	void register_null_memory_resource()
	{
		tester.register_test("NullMemoryResource#yzz2zm9bqm3j4xvs", test_yzz2zm9bqm3j4xvs);
	}

	void register_new_delete_memory_resource()
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
