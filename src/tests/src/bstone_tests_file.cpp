#include <algorithm>
#include <iterator>

#include "bstone_file.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// File(const char*, FileOpenMode)
void test_9yco8uvvyhuadh0b()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		const auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// File(const char*)
void test_k6rgtthwam1unqvv()
{
	auto is_created = false;
	auto is_opened = false;

	try
	{
		const auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_created = file.is_open();
	}
	catch (...) {}

	try
	{
		const auto file = bstone::File{"test.data"};
		is_opened = file.is_open();
	}
	catch (...) {}

	tester.check(is_created && is_opened);
}

// ==========================================================================

// open(const char*, FileOpenMode)
void test_dftddy2zdkyxexy5()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{};
		file.open("test.data", bstone::FileOpenMode::create);
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// open(const char*)
void test_e0c9zclt1z6cw2b6()
{
	auto is_created = false;
	auto is_opened = false;

	try
	{
		const auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_created = file.is_open();
	}
	catch (...) {}

	try
	{
		auto file = bstone::File{};
		file.open("test.data");
		is_opened = file.is_open();
	}
	catch (...) {}

	tester.check(is_created && is_opened);
}

// ==========================================================================

// close()
void test_x1b4q9amjfo7wd3r()
{
	auto is_failed = false;
	auto is_created = false;
	auto is_closed = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_created = file.is_open();
		file.close();
		is_closed = !file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_created && is_closed);
}

// ==========================================================================

// is_open()
void test_kyiqeenrhj4s9vdm()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		const auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// ==========================================================================

// read(void*, IntP) and write(const void*, IntP)
void test_fmo05coq6xfc71cv()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_zero_size = false;
	auto is_written = false;
	auto is_set_position = false;
	auto is_read = false;
	auto is_equal = false;

	try
	{
		constexpr auto byte_count = 4;
		const unsigned char ref_bytes[byte_count] = {0x11, 0x44, 0x22, 0x33};

		unsigned char bytes[byte_count] = {};

		auto file = bstone::File
		{
			"test.data",
			bstone::FileOpenMode::create | bstone::FileOpenMode::truncate | bstone::FileOpenMode::read
		};

		is_open = file.is_open();
		is_zero_size = file.get_size() == 0;
		is_written = file.write(ref_bytes + 0, 2) == 2 && file.write(ref_bytes + 2, 2) == 2;
		is_set_position = file.set_position(0, bstone::FileOrigin::begin) == 0;
		is_read = file.read(bytes + 0, 2) == 2 && file.read(bytes + 2, 2) == 2;
		is_equal = std::equal(std::cbegin(bytes), std::cend(bytes), ref_bytes);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_zero_size && is_written && is_set_position && is_read && is_equal);
}

// ==========================================================================

// set_position(Int64, FileOrigin)
void test_fi6d3dq5xqlmagzn()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;
	auto is_set_position_3 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
		file.set_size(100);
		is_set_position_1 = file.set_position(0, bstone::FileOrigin::end) == 100;
		is_set_position_2 = file.set_position(10, bstone::FileOrigin::begin) == 10;
		is_set_position_3 = file.set_position(-5, bstone::FileOrigin::current) == 5;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_set_position_1 && is_set_position_2 && is_set_position_3);
}

// ==========================================================================

// flush()
void test_4d4l6vgdguqfjarx()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
		file.flush();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_file();
		register_open();
		register_close();
		register_is_open();
		register_read_write();
		register_set_position();
		register_flush();
	}

private:
	void register_file()
	{
		tester.register_test("File#9yco8uvvyhuadh0b", test_9yco8uvvyhuadh0b);
		tester.register_test("File#k6rgtthwam1unqvv", test_k6rgtthwam1unqvv);
	}

	void register_open()
	{
		tester.register_test("File#dftddy2zdkyxexy5", test_dftddy2zdkyxexy5);
		tester.register_test("File#e0c9zclt1z6cw2b6", test_e0c9zclt1z6cw2b6);
	}

	void register_close()
	{
		tester.register_test("File#x1b4q9amjfo7wd3r", test_x1b4q9amjfo7wd3r);
	}

	void register_is_open()
	{
		tester.register_test("File#kyiqeenrhj4s9vdm", test_kyiqeenrhj4s9vdm);
	}

	void register_read_write()
	{
		tester.register_test("File#fmo05coq6xfc71cv", test_fmo05coq6xfc71cv);
	}

	void register_set_position()
	{
		tester.register_test("File#fi6d3dq5xqlmagzn", test_fi6d3dq5xqlmagzn);
	}

	void register_flush()
	{
		tester.register_test("File#4d4l6vgdguqfjarx", test_4d4l6vgdguqfjarx);
	}
};

auto registrator = Registrator{};

} // namespace
