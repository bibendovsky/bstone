#include <string>

#include "bstone_tester.h"

#include "bstone_source_location.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// SourceLocation()
void test_jjkfeq1h0eky5v3j()
{
	constexpr auto source_location = bstone::SourceLocation{};
	constexpr auto file_name = source_location.get_file_name();
	constexpr auto line = source_location.get_line();
	constexpr auto function_name = source_location.get_function_name();

	tester.check(
		file_name != nullptr && *file_name == '\0' &&
		line == 0 &&
		function_name != nullptr && *function_name == '\0');
}

// SourceLocation(const char*, int, const char*)
void test_yvjcwelc9in2xx36()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// SourceLocation(const SourceLocation&)
void test_tjba0zn4uxwrsjbh()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location_1 = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	auto source_location = source_location_1;

	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// operator=(const SourceLocation&)
void test_ghc62jbn00k18o6o()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location_1 = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	auto source_location = bstone::SourceLocation{};
	source_location = source_location_1;

	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// get_file_name()
void test_pcbm2fxmrd6dl9xo()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto file_name = source_location.get_file_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name);
}

// get_line()
void test_wdklrjvi6l8dovv1()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto line = source_location.get_line();

	tester.check(line == reference_line);
}

// get_function_name()
void test_69ysln16wwcpje0s()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	const auto source_location = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	const auto function_name = source_location.get_function_name();

	tester.check(
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

void test_yw3n21if0my351id()
{
	const auto reference_file_name = std::string{"file_name"};
	constexpr auto reference_line = 1'234'567'890;
	const auto reference_function_name = std::string{"function_name"};

	auto source_location_1 = bstone::SourceLocation{
		reference_file_name.c_str(),
		reference_line,
		reference_function_name.c_str()};

	auto source_location = bstone::SourceLocation{"???", 0, "???"};
	source_location.swap(source_location_1);

	const auto file_name = source_location.get_file_name();
	const auto line = source_location.get_line();
	const auto function_name = source_location.get_function_name();

	tester.check(
		file_name == reference_file_name.c_str() &&
		std::string{file_name} == reference_file_name &&
		line == reference_line &&
		function_name == reference_function_name.c_str() &&
		std::string{function_name} == reference_function_name);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		tester.register_test("SourceLocation#jjkfeq1h0eky5v3j", test_jjkfeq1h0eky5v3j);
		tester.register_test("SourceLocation#yvjcwelc9in2xx36", test_yvjcwelc9in2xx36);
		tester.register_test("SourceLocation#wdklrjvi6l8dovv1", test_tjba0zn4uxwrsjbh);
		tester.register_test("SourceLocation#ghc62jbn00k18o6o", test_ghc62jbn00k18o6o);
		tester.register_test("SourceLocation#69ysln16wwcpje0s", test_pcbm2fxmrd6dl9xo);
		tester.register_test("SourceLocation#wdklrjvi6l8dovv1", test_wdklrjvi6l8dovv1);
		tester.register_test("SourceLocation#69ysln16wwcpje0s", test_69ysln16wwcpje0s);
		tester.register_test("SourceLocation#yw3n21if0my351id", test_yw3n21if0my351id);
	}
};

auto registrator = Registrator{};

} // namespace
