#include <utility>

#include "bstone_process.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto file_path =
	"."
#ifdef _WIN32
	"\\"
#else
	"/"
#endif
	"bstone_tests_process.bin"
	;

constexpr auto invalid_file_path = "/:";
constexpr auto invalid_url = "???";

constexpr auto ref_exit_code = 42;
constexpr auto ref_with_space_exit_code = 43;
constexpr auto ref_url = "http://info.cern.ch/hypertext/WWW/TheProject.html";

// ==========================================================================

// int create_and_wait_for_exit(const CreateAndWaitForExitParam&)
void test_5p1ropw65rwl05pg()
{
	const char* args[] = {file_path};

	auto param = bstone::process::CreateAndWaitForExitParam{};
	param.argc = 1;
	param.argv = args;
	param.working_directory = ".";

	const auto result = bstone::process::create_and_wait_for_exit(param);
	const auto is_valid = result.has_exit_code && result.exit_code == ref_exit_code;
	tester.check(is_valid);
}

// int create_and_wait_for_exit(const CreateAndWaitForExitParam&)
// Argument with space.
void test_crqda9voeeft5tsz()
{
	const char* args[] = {file_path, "with space"};

	auto param = bstone::process::CreateAndWaitForExitParam{};
	param.argc = 2;
	param.argv = args;
	param.working_directory = ".";

	const auto result = bstone::process::create_and_wait_for_exit(param);
	const auto is_valid = result.has_exit_code && result.exit_code == ref_with_space_exit_code;
	tester.check(is_valid);
}

// int create_and_wait_for_exit(const CreateAndWaitForExitParam&)
// No args.
void test_sa1zhnmlqvwlsv7w()
{
	auto param = bstone::process::CreateAndWaitForExitParam{};
	param.working_directory = ".";

	auto is_failed = false;

	try
	{
		bstone::process::create_and_wait_for_exit(param);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// int create_and_wait_for_exit(const CreateAndWaitForExitParam&)
// Invalid file path.
void test_h7c0llq3kh1my1bl()
{
	const char* args[] = {invalid_file_path};

	auto param = bstone::process::CreateAndWaitForExitParam{};
	param.argc = 1;
	param.argv = args;
	param.working_directory = ".";

	auto is_failed = false;

	try
	{
		bstone::process::create_and_wait_for_exit(param);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// int create_and_wait_for_exit(const CreateAndWaitForExitParam&)
// No exit code.
void test_pxly3lq6nhn356x6()
{
	const char* args[] = {file_path, "abort"};

	auto param = bstone::process::CreateAndWaitForExitParam{};
	param.argc = 2;
	param.argv = args;
	param.working_directory = ".";

	const auto result = bstone::process::create_and_wait_for_exit(param);
	const auto is_valid = !result.has_exit_code;

	tester.check(is_valid);
}

// ==========================================================================

// void open_file_or_url(const char*)
void test_ehx0pjs8b3qti571()
{
	bstone::process::open_file_or_url(ref_url);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_create_and_wait_for_exit();

		register_open_file_or_url();
	}

private:
	void register_create_and_wait_for_exit()
	{
		tester.register_test("process::create_and_wait_for_exit#5p1ropw65rwl05pg", test_5p1ropw65rwl05pg);
		tester.register_test("process::create_and_wait_for_exit#crqda9voeeft5tsz", test_crqda9voeeft5tsz);
		tester.register_test("process::create_and_wait_for_exit#sa1zhnmlqvwlsv7w", test_sa1zhnmlqvwlsv7w);
		tester.register_test("process::create_and_wait_for_exit#h7c0llq3kh1my1bl", test_h7c0llq3kh1my1bl);
		tester.register_test("process::create_and_wait_for_exit#pxly3lq6nhn356x6", test_pxly3lq6nhn356x6);
	}

	void register_open_file_or_url()
	{
		tester.register_test("process::open_file_or_url#ehx0pjs8b3qti571", test_ehx0pjs8b3qti571);
	}
};

auto registrator = Registrator{};

} // namespace
