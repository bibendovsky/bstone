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
		is_set_position = file.seek(0, bstone::FileOrigin::begin) == 0;
		is_read = file.read(bytes + 0, 2) == 2 && file.read(bytes + 2, 2) == 2;
		is_equal = std::equal(std::cbegin(bytes), std::cend(bytes), ref_bytes);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_zero_size && is_written && is_set_position && is_read && is_equal);
}

// IntP read(void*, IntP)
// Closed.
void test_6mmfx7oqkvu46lmo()
{
	auto is_failed = false;
	auto file = bstone::File{};

	try
	{
		char buffer;
		file.read(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// IntP write(const void*, IntP)
// Closed.
void test_32hb8m11bqdomfys()
{
	auto is_failed = false;
	auto file = bstone::File{};

	try
	{
		const auto buffer = '\0';
		file.write(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// `void read_exact(void*, IntP)` and `void write_exact(const void*, IntP)`
void test_0vj5qawe151dlab8()
{
	auto is_open = false;
	auto is_zero_size = false;
	auto is_set_position = false;
	auto is_equal = false;

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
	file.write_exact(ref_bytes + 0, 2);
	file.write_exact(ref_bytes + 2, 2);
	is_set_position = file.seek(0, bstone::FileOrigin::begin) == 0;
	file.read_exact(bytes + 0, 2);
	file.read_exact(bytes + 2, 2);
	is_equal = std::equal(std::cbegin(bytes), std::cend(bytes), ref_bytes);

	tester.check(is_open && is_zero_size && is_set_position && is_equal);
}

// void read_exact(void*, IntP)
// Fail.
void test_ip74fy4x4vq54ql3()
{
	auto file = bstone::File
	{
		"test.data",
		bstone::FileOpenMode::create | bstone::FileOpenMode::truncate | bstone::FileOpenMode::read
	};

	auto buffer = '\0';
	auto is_failed = false;

	try
	{
		file.read_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void read_exact(void*, IntP)
// Closed.
void test_wim9y9uexr2x18xc()
{
	auto file = bstone::File{};
	char buffer;
	auto is_failed = false;

	try
	{
		file.read_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void write_exact(const void*, IntP)
// Closed.
void test_ko6g2vzhmpoxkt3j()
{
	auto file = bstone::File{};
	const auto buffer = '\0';
	auto is_failed = false;

	try
	{
		file.write_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// seek(Int64, FileOrigin)
void test_1ywq7j1zp67guwip()
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
		is_set_position_1 = file.seek(0, bstone::FileOrigin::end) == 100;
		is_set_position_2 = file.seek(10, bstone::FileOrigin::begin) == 10;
		is_set_position_3 = file.seek(-5, bstone::FileOrigin::current) == 5;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_set_position_1 && is_set_position_2 && is_set_position_3);
}

// seek(Int64, FileOrigin)
// Fail new position.
void test_g19x58du9tm8l39s()
{
	auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.seek(-5, bstone::FileOrigin::current);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// seek(Int64, FileOrigin)
// Closed.
void test_c4xnnx60bh0el6c0()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.seek(0, bstone::FileOrigin::begin);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// Int64 skip(Int64)
void test_7flwx51n13c9137h()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
		is_set_position_1 = file.skip(100) == 100;
		is_set_position_2 = file.seek(0, bstone::FileOrigin::current) == 100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_set_position_1 && is_set_position_2);
}

// Int64 skip(Int64)
// Fail new position.
void test_ykqf55ec945bin2j()
{
	auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.skip(-5);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// Int64 skip(Int64)
// Closed.
void test_rwvrfvlzq257vhm6()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.skip(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// Int64 get_position()
void test_0gj54wvhksw88ae5()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
		is_set_position_1 = file.seek(100, bstone::FileOrigin::begin) == 100;
		is_set_position_2 = file.get_position() == 100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_set_position_1 && is_set_position_2);
}

// Int64 get_position()
// Closed.
void test_nthguyes1qfzyjph()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.get_position();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// void set_position(Int64)
void test_ejj6s2bvcn5ba767()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
		is_open = file.is_open();
		file.set_position(100);
		is_set_position = file.seek(0, bstone::FileOrigin::current) == 100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_set_position);
}

// void set_position(Int64)
// Fail new position.
void test_wh4ro8o3yw0djyc6()
{
	auto file = bstone::File{"test.data", bstone::FileOpenMode::create};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.set_position(-5);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void set_position(Int64)
// Closed.
void test_15p3gi1ajh8vz0x1()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	auto is_failed = false;

	try
	{
		file.set_position(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// Int64 get_size() const
void test_q5bqlob75o008k21()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_valid_size_1 = false;
	auto is_valid_size_2 = false;

	try
	{
		const char buffer = '\0';
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create | bstone::FileOpenMode::truncate};
		is_open = file.is_open();
		is_valid_size_1 = file.get_size() == 0;
		file.seek(99, bstone::FileOrigin::begin);
		file.write(&buffer, 1);
		is_valid_size_2 = file.get_size() == 100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_valid_size_1 && is_valid_size_2);
}

// ==========================================================================

// void set_size(Int64) const
void test_fuyi2tfqnsxirj3b()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_valid_size_1 = false;
	auto is_valid_size_2 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenMode::create | bstone::FileOpenMode::truncate};
		is_open = file.is_open();
		is_valid_size_1 = file.get_size() == 0;
		file.set_size(100);
		is_valid_size_2 = file.get_size() == 100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open && is_valid_size_1 && is_valid_size_2);
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
		register_read_exact_write_exact();
		register_seek();
		register_skip();
		register_get_position();
		register_set_position();
		register_get_size();
		register_set_size();
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
		tester.register_test("File#6mmfx7oqkvu46lmo", test_6mmfx7oqkvu46lmo);
		tester.register_test("File#32hb8m11bqdomfys", test_32hb8m11bqdomfys);
	}

	void register_read_exact_write_exact()
	{
		tester.register_test("File#0vj5qawe151dlab8", test_0vj5qawe151dlab8);
		tester.register_test("File#ip74fy4x4vq54ql3", test_ip74fy4x4vq54ql3);
		tester.register_test("File#wim9y9uexr2x18xc", test_wim9y9uexr2x18xc);
		tester.register_test("File#ko6g2vzhmpoxkt3j", test_ko6g2vzhmpoxkt3j);
	}

	void register_seek()
	{
		tester.register_test("File#1ywq7j1zp67guwip", test_1ywq7j1zp67guwip);
		tester.register_test("File#g19x58du9tm8l39s", test_g19x58du9tm8l39s);
		tester.register_test("File#c4xnnx60bh0el6c0", test_c4xnnx60bh0el6c0);
	}

	void register_skip()
	{
		tester.register_test("File#7flwx51n13c9137h", test_7flwx51n13c9137h);
		tester.register_test("File#ykqf55ec945bin2j", test_ykqf55ec945bin2j);
		tester.register_test("File#rwvrfvlzq257vhm6", test_rwvrfvlzq257vhm6);
	}

	void register_get_position()
	{
		tester.register_test("File#0gj54wvhksw88ae5", test_0gj54wvhksw88ae5);
		tester.register_test("File#nthguyes1qfzyjph", test_nthguyes1qfzyjph);
	}

	void register_set_position()
	{
		tester.register_test("File#ejj6s2bvcn5ba767", test_ejj6s2bvcn5ba767);
		tester.register_test("File#wh4ro8o3yw0djyc6", test_wh4ro8o3yw0djyc6);
		tester.register_test("File#15p3gi1ajh8vz0x1", test_15p3gi1ajh8vz0x1);
	}

	void register_get_size()
	{
		tester.register_test("File#q5bqlob75o008k21", test_q5bqlob75o008k21);
	}

	void register_set_size()
	{
		tester.register_test("File#fuyi2tfqnsxirj3b", test_fuyi2tfqnsxirj3b);
	}

	void register_flush()
	{
		tester.register_test("File#4d4l6vgdguqfjarx", test_4d4l6vgdguqfjarx);
	}
};

auto registrator = Registrator{};

} // namespace
