#if defined(_WIN32)

#include <algorithm>

#include "bstone_tester.h"
#include "bstone_win32_wstring.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// Win32WString()
void test_sqsflg2h72usjet9()
{
	const auto string = bstone::Win32WString{};
	const auto size = string.get_size();
	const auto data = string.get_data();

	tester.check(size == 0 && data == nullptr);
}

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
		tester.register_test("Win32WString#sqsflg2h72usjet9", test_sqsflg2h72usjet9);
		tester.register_test("Win32WString#tg392qlci41gqhvg", test_tg392qlci41gqhvg);
	}
};

auto registrator = Registrator{};

} // namespace

#endif // _WIN32
