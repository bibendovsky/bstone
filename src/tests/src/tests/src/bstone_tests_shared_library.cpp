#include <utility>

#include "bstone_shared_library.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

constexpr auto file_path =
	"."
#if defined(_WIN32)
	"\\"
#else
	"/"
#endif
	"bstone_tests_shared_library.bin"
	;

constexpr auto symbol_name = "api_function";

// ==========================================================================

// SharedLibrary()
void test_ubplanyvz16awucd()
{
	const auto shared_library = bstone::SharedLibrary{};
	const auto is_open = shared_library.is_open();
	tester.check(!is_open);
}

// SharedLibrary(SharedLibrary&&) noexcept
void test_jz3oev0ib4s0jpax()
{
	auto shared_library_1 = bstone::SharedLibrary{file_path};
	const auto shared_library_2 = std::move(shared_library_1);
	tester.check(shared_library_2.is_open());
}

// SharedLibrary& operator=(SharedLibrary&&) noexcept
void test_5pb46ml8e2h0tni3()
{
	auto shared_library_1 = bstone::SharedLibrary{file_path};
	auto shared_library_2 = bstone::SharedLibrary{};
	shared_library_2 = std::move(shared_library_1);
	tester.check(shared_library_2.is_open());
}

// SharedLibrary(const char*)
void test_3l83odzn2i3d7fld()
{
	const auto shared_library = bstone::SharedLibrary{file_path};
	const auto is_open = shared_library.is_open();
	tester.check(is_open);
}

// ==========================================================================

// bool is_open() const noexcept
void test_qtsqg53k7svkyznb()
{
	const auto shared_library = bstone::SharedLibrary{};
	const auto is_open = shared_library.is_open();
	tester.check(!is_open);
}

// ==========================================================================

// bool try_open(const char*)
void test_eupfy1p3a3d4j5ez()
{
	auto shared_library = bstone::SharedLibrary{};
	const auto is_open_1 = shared_library.is_open();
	const auto is_open_2 = shared_library.try_open(file_path);
	tester.check(!is_open_1 && is_open_2);
}

// bool try_open(const char*)
// Fail.
void test_1emzwqqr4t98bxwo()
{
	auto shared_library = bstone::SharedLibrary{};
	const auto is_open = shared_library.try_open(".");
	tester.check(!is_open);
}

// ==========================================================================

// void open(const char*)
void test_8sek7gwvmhg1zeso()
{
	auto shared_library = bstone::SharedLibrary{};
	const auto is_open_1 = shared_library.is_open();
	shared_library.open(file_path);
	const auto is_open_2 = shared_library.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// void open(const char*)
// Fail.
void test_ydft4iw452irg60f()
{
	auto is_failed = false;

	try
	{
		const auto shared_library = bstone::SharedLibrary{"."};
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// void close() noexcept
void test_jd023d4h8jxu113v()
{
	auto shared_library = bstone::SharedLibrary{file_path};
	const auto is_open_1 = shared_library.is_open();
	shared_library.close();
	const auto is_open_2 = shared_library.is_open();
	tester.check(is_open_1 && !is_open_2);
}

// ==========================================================================

// void* find_symbol(const char*) const noexcept
void test_qwq5sx3u0ea3su55()
{
	const auto shared_library = bstone::SharedLibrary{file_path};
	const auto symbol = shared_library.find_symbol(symbol_name);
	tester.check(symbol != nullptr);
}

// void* find_symbol(const char*) const noexcept
// Fail.
void test_3u0h00y8xna7me1y()
{
	const auto shared_library = bstone::SharedLibrary{file_path};
	const auto symbol = shared_library.find_symbol("");
	tester.check(symbol == nullptr);
}

// template<typename T>
// T find_symbol(const char*) const noexcept
void test_ut1z994z5n1hqgts()
{
	using Func = int (*)();

	const auto shared_library = bstone::SharedLibrary{file_path};
	const auto symbol = shared_library.find_symbol<Func>(symbol_name);

	constexpr auto ref_func_result = 42;
	auto func_result = 0;

	if (symbol != nullptr)
	{
		func_result = symbol();
	}

	tester.check(symbol != nullptr && func_result == ref_func_result);
}

// template<typename T>
// T find_symbol(const char*) const noexcept
// Fail.
void test_cf0g2vruliszvalb()
{
	using Func = int (*)();

	const auto shared_library = bstone::SharedLibrary{file_path};
	const auto symbol = shared_library.find_symbol<Func>("");
	tester.check(symbol == nullptr);
}

// ==========================================================================

// swap(SharedLibrary&) noexcept
void test_j2dsjk32qc1ezush()
{
	auto shared_library_1 = bstone::SharedLibrary{file_path};
	auto shared_library_2 = bstone::SharedLibrary{};
	shared_library_2.swap(shared_library_1);

	const auto is_valid_1 = !shared_library_1.is_open();
	const auto is_valid_2 = shared_library_2.is_open();

	const auto is_valid =
		is_valid_1 &&
		is_valid_2 &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_shared_library();
		register_is_open();
		register_try_open();
		register_open();
		register_close();
		register_find_symbol();
		register_swap();
	}

private:
	void register_shared_library()
	{
		tester.register_test("SharedLibrary#ubplanyvz16awucd", test_ubplanyvz16awucd);
		tester.register_test("SharedLibrary#jz3oev0ib4s0jpax", test_jz3oev0ib4s0jpax);
		tester.register_test("SharedLibrary#5pb46ml8e2h0tni3", test_5pb46ml8e2h0tni3);
		tester.register_test("SharedLibrary#3l83odzn2i3d7fld", test_3l83odzn2i3d7fld);
	}

	void register_is_open()
	{
		tester.register_test("SharedLibrary#qtsqg53k7svkyznb", test_qtsqg53k7svkyznb);
	}

	void register_try_open()
	{
		tester.register_test("SharedLibrary#eupfy1p3a3d4j5ez", test_eupfy1p3a3d4j5ez);
		tester.register_test("SharedLibrary#1emzwqqr4t98bxwo", test_1emzwqqr4t98bxwo);
	}

	void register_open()
	{
		tester.register_test("SharedLibrary#8sek7gwvmhg1zeso", test_8sek7gwvmhg1zeso);
		tester.register_test("SharedLibrary#ydft4iw452irg60f", test_ydft4iw452irg60f);
	}

	void register_close()
	{
		tester.register_test("SharedLibrary#jd023d4h8jxu113v", test_jd023d4h8jxu113v);
	}

	void register_find_symbol()
	{
		tester.register_test("SharedLibrary#qwq5sx3u0ea3su55", test_qwq5sx3u0ea3su55);
		tester.register_test("SharedLibrary#3u0h00y8xna7me1y", test_3u0h00y8xna7me1y);
		tester.register_test("SharedLibrary#ut1z994z5n1hqgts", test_ut1z994z5n1hqgts);
		tester.register_test("SharedLibrary#cf0g2vruliszvalb", test_cf0g2vruliszvalb);
	}

	void register_swap()
	{
		tester.register_test("SharedLibrary#j2dsjk32qc1ezush", test_j2dsjk32qc1ezush);
	}
};

auto registrator = Registrator{};

} // namespace
