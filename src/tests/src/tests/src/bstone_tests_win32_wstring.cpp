#if defined(_WIN32)

#include <algorithm>
#include <memory>

#include "bstone_tester.h"
#include "bstone_win32_wstring.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto memory_fill_value = 'H';

class MemoryResource final : public bstone::MemoryResource
{
public:
	MemoryResource() = default;
	~MemoryResource() override = default;

private:
	void* do_allocate(std::intptr_t size) override
	{
		const auto ptr = bstone::get_default_memory_resource().allocate(size);
		std::fill_n(static_cast<char*>(ptr), size, memory_fill_value);
		return ptr;
	}

	void do_deallocate(void* ptr) noexcept override
	{
		bstone::get_default_memory_resource().deallocate(ptr);
	}
};

auto memory_resource = MemoryResource{};

// ==========================================================================

// Win32WString()
void test_h45dvefmdtetvvow()
{
	const auto string = bstone::Win32WString{};

	const auto result_1 = string.get_capacity();
	const auto is_valid_1 = result_1 == 0;

	const auto result_2 = string.get_size();
	const auto is_valid_2 = result_2 == 0;

	const auto result_3 = string.get_data();
	const auto is_valid_3 = result_3 == nullptr;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// Win32WString(MemoryResource&)
void test_ivlht4ejlr2shq5a()
{
	const auto string = bstone::Win32WString{memory_resource};

	const auto result_1 = string.get_capacity();
	const auto is_valid_1 = result_1 == 0;

	const auto result_2 = string.get_size();
	const auto is_valid_2 = result_2 == 0;

	const auto result_3 = string.get_data();
	const auto is_valid_3 = result_3 == nullptr;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// Win32WString(std::intptr_t)
void test_2fmktd9boq97qc7s()
{
	constexpr auto ref_capacity = std::intptr_t{2};

	const auto string = bstone::Win32WString{ref_capacity};

	const auto result_1 = string.get_capacity();
	const auto is_valid_1 = result_1 == ref_capacity;

	const auto result_2 = string.get_size();
	const auto is_valid_2 = result_2 == 0;

	const auto result_3 = string.get_data();
	const auto is_valid_3 = result_3 != nullptr;

	const auto octets = reinterpret_cast<const char*>(string.get_data());

	const auto is_valid_4 =
		octets[0] == '\0' &&
		octets[1] == '\0' &&
		true;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// Win32WString(std::intptr_t, MemoryResource&)
void test_sqsflg2h72usjet9()
{
	constexpr auto ref_capacity = std::intptr_t{2};

	const auto string = bstone::Win32WString{ref_capacity, memory_resource};

	const auto result_1 = string.get_capacity();
	const auto is_valid_1 = result_1 == ref_capacity;

	const auto result_2 = string.get_size();
	const auto is_valid_2 = result_2 == 0;

	const auto result_3 = string.get_data();
	const auto is_valid_3 = result_3 != nullptr;

	const auto octets = reinterpret_cast<const char*>(string.get_data());

	const auto is_valid_4 =
		octets[0] == '\0' &&
		octets[1] == '\0' &&
		octets[2] == memory_fill_value &&
		octets[3] == memory_fill_value &&
		true;

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		is_valid_3 &&
		is_valid_4 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// Win32WString(const char*)
void test_tg392qlci41gqhvg()
{
	const wchar_t* ref_string_1 = L"string";
	const auto string_1 = bstone::Win32WString{"string"};

	const wchar_t* ref_string_2 = L"STRING";
	const auto string_2 = bstone::Win32WString{"STRING"};

	const auto size_1 = string_1.get_size();
	const auto data_1 = string_1.get_data();
	const auto are_equal_1 = std::equal(data_1, data_1 + size_1, ref_string_1);

	const auto size_2 = string_2.get_size();
	const auto data_2 = string_2.get_data();
	const auto are_equal_2 = std::equal(data_2, data_2 + size_2, ref_string_2);

	tester.check(size_1 == 6 && are_equal_1 && size_2 == 6 && are_equal_2);
}

// ==========================================================================

// Win32WString(const char*, MemoryResource&)
void test_0qmmhvhvqeqzrbcs()
{
	const wchar_t* ref_string_1 = L"string";
	const auto string_1 = bstone::Win32WString{"string", memory_resource};

	const wchar_t* ref_string_2 = L"STRING";
	const auto string_2 = bstone::Win32WString{"STRING", memory_resource};

	const auto size_1 = string_1.get_size();
	const auto data_1 = string_1.get_data();
	const auto are_equal_1 = std::equal(data_1, data_1 + size_1, ref_string_1);

	const auto size_2 = string_2.get_size();
	const auto data_2 = string_2.get_data();
	const auto are_equal_2 = std::equal(data_2, data_2 + size_2, ref_string_2);

	tester.check(size_1 == 6 && are_equal_1 && size_2 == 6 && are_equal_2);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_win32_wstring();
	}

private:
	void register_win32_wstring()
	{
		tester.register_test("Win32WString#h45dvefmdtetvvow", test_h45dvefmdtetvvow);
		tester.register_test("Win32WString#ivlht4ejlr2shq5a", test_ivlht4ejlr2shq5a);
		tester.register_test("Win32WString#2fmktd9boq97qc7s", test_2fmktd9boq97qc7s);
		tester.register_test("Win32WString#sqsflg2h72usjet9", test_sqsflg2h72usjet9);
		tester.register_test("Win32WString#tg392qlci41gqhvg", test_tg392qlci41gqhvg);
		tester.register_test("Win32WString#0qmmhvhvqeqzrbcs", test_0qmmhvhvqeqzrbcs);
	}
};

auto registrator = Registrator{};

} // namespace

#endif // _WIN32
