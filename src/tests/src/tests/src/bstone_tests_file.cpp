#include <algorithm>
#include <fstream>
#include <iterator>

#include "bstone_file.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

void create_empty_test_data_file()
{
	const auto stream_flags = std::ios_base::out | std::ios_base::binary | std::ios_base::trunc;
	const auto stream = std::ofstream{"test.data", stream_flags};
}

// ==========================================================================

// File()
void test_k6nu38pkrj6i0kdj()
{
	const auto file = bstone::File{};
	const auto is_open = file.is_open();
	tester.check(!is_open);
}

// ==========================================================================

// File(const char*, FileOpenFlags)
void test_9yco8uvvyhuadh0b()
{
	const auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
	const auto is_open = file.is_open();
	tester.check(is_open);
}

// File(const char*, FileOpenFlags)
// Invalid flags.
void test_wt113y5sf90zhymy()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		const auto file = bstone::File{"test.data", bstone::FileOpenFlags::none};
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// ==========================================================================

// File(const char*)
void test_k6rgtthwam1unqvv()
{
	create_empty_test_data_file();
	const auto file = bstone::File{"test.data"};
	const auto is_open = file.is_open();
	tester.check(is_open);
}

// ==========================================================================

// bool try_open(const char*, FileOpenFlags)
void test_zc5nebuu4x529qf0()
{
	auto file = bstone::File{};
	const auto try_is_open = file.try_open("test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive);
	const auto is_open = file.is_open();
	tester.check(try_is_open && is_open);
}

// bool try_open(const char*, FileOpenFlags)
// Invalid flags.
void test_5c7kqca20aoesdq4()
{
	auto file = bstone::File{};
	auto is_failed = false;
	auto try_is_open = false;
	auto is_open = false;

	try
	{
		try_is_open = file.try_open("test.data", bstone::FileOpenFlags::none);
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !try_is_open && !is_open);
}

// ==========================================================================

// bool try_open(const char*)
void test_l61nh9xx77tqnxxy()
{
	create_empty_test_data_file();

	auto file = bstone::File{};
	const auto try_is_open = file.try_open("test.data");
	const auto is_open = file.is_open();
	tester.check(try_is_open && is_open);
}

// ==========================================================================

// open(const char*, FileOpenFlags)
void test_dftddy2zdkyxexy5()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{};
		file.open("test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive);
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// open(const char*, FileOpenFlags)
// Invalid flags.
void test_1xzi20ioh2lw7ohh()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{};
		file.open("test.data", bstone::FileOpenFlags::none);
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// ==========================================================================

// open(const char*)
void test_e0c9zclt1z6cw2b6()
{
	create_empty_test_data_file();

	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{};
		file.open("test.data");
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// ==========================================================================

// close() noexcept
void test_x1b4q9amjfo7wd3r()
{
	auto is_failed = false;
	auto is_created = false;
	auto is_closed = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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
		const auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
		is_open = file.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// ==========================================================================

// read(void*, std::intptr_t) and write(const void*, std::intptr_t)
void test_fmo05coq6xfc71cv()
{
	auto is_open = false;
	auto is_zero_size = false;
	auto is_written = false;
	auto is_set_position = false;
	auto is_read = false;
	auto is_equal = false;

	constexpr auto byte_count = 4;
	const unsigned char ref_bytes[byte_count] = {0x11, 0x44, 0x22, 0x33};

	unsigned char bytes[byte_count] = {};

	auto file = bstone::File
	{
		"test.data",
		bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate | bstone::FileOpenFlags::read,
		bstone::FileShareMode::exclusive
	};

	is_open = file.is_open();
	is_zero_size = file.get_size() == 0;
	is_written = file.write(ref_bytes + 0, 2) == 2 && file.write(ref_bytes + 2, 2) == 2;
	is_set_position = file.seek(0, bstone::FileOrigin::begin) == 0;
	is_read = file.read(bytes + 0, 2) == 2 && file.read(bytes + 2, 2) == 2;
	is_equal = std::equal(std::cbegin(bytes), std::cend(bytes), ref_bytes);

	tester.check(is_open && is_zero_size && is_written && is_set_position && is_read && is_equal);
}

// read(void*, std::intptr_t)
// Closed.
void test_7wf8wo8vytpafln8()
{
	auto is_failed = false;
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	try
	{
		auto buffer = '\0';
		file.read(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// write(const void*, std::intptr_t)
// Closed.
void test_pg23loxd7hva5ty7()
{
	auto is_failed = false;

	auto file = bstone::File{};
	const auto is_open = file.is_open();

	try
	{
		const auto buffer = '\0';
		file.write(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// `void read_exactly(void*, std::intptr_t)` and `void write_exactly(const void*, std::intptr_t)`
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
		bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate | bstone::FileOpenFlags::read,
		bstone::FileShareMode::exclusive
	};

	is_open = file.is_open();
	is_zero_size = file.get_size() == 0;
	file.write_exactly(ref_bytes + 0, 2);
	file.write_exactly(ref_bytes + 2, 2);
	is_set_position = file.seek(0, bstone::FileOrigin::begin) == 0;
	file.read_exactly(bytes + 0, 2);
	file.read_exactly(bytes + 2, 2);
	is_equal = std::equal(std::cbegin(bytes), std::cend(bytes), ref_bytes);

	tester.check(is_open && is_zero_size && is_set_position && is_equal);
}

// void read_exactly(void*, std::intptr_t)
// Mismatch count.
void test_ip74fy4x4vq54ql3()
{
	auto file = bstone::File
	{
		"test.data",
		bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate | bstone::FileOpenFlags::read,
		bstone::FileShareMode::exclusive
	};

	auto buffer = '\0';
	auto is_failed = false;

	try
	{
		file.read_exactly(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void read_exactly(void*, std::intptr_t)
// Closed.
void test_h2ldps4weu4ia2s6()
{
	auto is_failed = false;
	auto file = bstone::File{};
	const auto is_open = file.is_open();

	try
	{
		auto buffer = '\0';
		file.read_exactly(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// void write_exactly(const void*, std::intptr_t)
// Closed.
void test_wjbtywqjl05sac7h()
{
	auto is_failed = false;

	auto file = bstone::File{};
	const auto is_open = file.is_open();

	try
	{
		auto buffer = '\0';
		file.write_exactly(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// ==========================================================================

// std::int64_t seek(std::int64_t, FileOrigin)
void test_1ywq7j1zp67guwip()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;
	auto is_set_position_3 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// std::int64_t seek(std::int64_t, FileOrigin)
// Closed.
void test_4r9qvyqhn1zlvzh8()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.seek(10, bstone::FileOrigin::begin);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// std::int64_t seek(std::int64_t, FileOrigin)
// Fail new position.
void test_g19x58du9tm8l39s()
{
	auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// ==========================================================================

// std::int64_t skip(std::int64_t)
void test_7flwx51n13c9137h()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// std::int64_t skip(std::int64_t)
// Closed.
void test_s8ifik5wzumorn6t()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.skip(100);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// ==========================================================================

// std::int64_t skip(std::int64_t)
// Fail new position.
void test_ykqf55ec945bin2j()
{
	auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// ==========================================================================

// std::int64_t get_position()
void test_0gj54wvhksw88ae5()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position_1 = false;
	auto is_set_position_2 = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// std::int64_t get_position()
// Closed.
void test_7gwwf9pezbag4zt5()
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

	tester.check(is_failed && !is_open);
}

// ==========================================================================

// void set_position(std::int64_t)
void test_ejj6s2bvcn5ba767()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_set_position = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// void set_position(std::int64_t)
// Closed.
void test_dqyslv8yuv2h8xpv()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.set_position(1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// void set_position(std::int64_t)
// Negative position.
void test_wh4ro8o3yw0djyc6()
{
	auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
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

// ==========================================================================

// std::int64_t get_size() const
void test_q5bqlob75o008k21()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_valid_size_1 = false;
	auto is_valid_size_2 = false;

	try
	{
		const char buffer = '\0';
		auto file = bstone::File{
			"test.data",
			bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate,
			bstone::FileShareMode::exclusive};
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

// std::int64_t get_size() const
// Closed.
void test_rt5sww69r9a7gj58()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.get_size();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// ==========================================================================

// void set_size(std::int64_t) const
void test_fuyi2tfqnsxirj3b()
{
	auto is_failed = false;
	auto is_open = false;
	auto is_valid_size_1 = false;
	auto is_valid_size_2 = false;

	try
	{
		auto file = bstone::File{
			"test.data",
			bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate,
			bstone::FileShareMode::exclusive};
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

// void set_size(std::int64_t) const
// Closed.
void test_dnpz1opo4nhy1irj()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.set_size(1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// void set_size(std::int64_t) const
// Negative size.
void test_5pgsxx7g674b55nh()
{
	auto file = bstone::File{
		"test.data",
		bstone::FileOpenFlags::create | bstone::FileOpenFlags::truncate,
		bstone::FileShareMode::exclusive};

	auto is_failed = false;
	const auto is_open = file.is_open();

	try
	{
		file.set_size(-1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && is_open);
}

// ==========================================================================

// void flush()
void test_4d4l6vgdguqfjarx()
{
	auto is_failed = false;
	auto is_open = false;

	try
	{
		auto file = bstone::File{"test.data", bstone::FileOpenFlags::create, bstone::FileShareMode::exclusive};
		is_open = file.is_open();
		file.flush();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_failed && is_open);
}

// void flush()
// Closed.
void test_6xxbxfl2qvej8l07()
{
	auto file = bstone::File{};
	const auto is_open = file.is_open();
	auto is_failed = false;

	try
	{
		file.flush();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed && !is_open);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_file();
		register_try_open();
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
		tester.register_test("File#k6nu38pkrj6i0kdj", test_k6nu38pkrj6i0kdj);
		tester.register_test("File#9yco8uvvyhuadh0b", test_9yco8uvvyhuadh0b);
		tester.register_test("File#wt113y5sf90zhymy", test_wt113y5sf90zhymy);
		tester.register_test("File#k6rgtthwam1unqvv", test_k6rgtthwam1unqvv);
	}

	void register_try_open()
	{
		tester.register_test("File#zc5nebuu4x529qf0", test_zc5nebuu4x529qf0);
		tester.register_test("File#5c7kqca20aoesdq4", test_5c7kqca20aoesdq4);
		tester.register_test("File#l61nh9xx77tqnxxy", test_l61nh9xx77tqnxxy);
	}

	void register_open()
	{
		tester.register_test("File#dftddy2zdkyxexy5", test_dftddy2zdkyxexy5);
		tester.register_test("File#1xzi20ioh2lw7ohh", test_1xzi20ioh2lw7ohh);
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
		tester.register_test("File#7wf8wo8vytpafln8", test_7wf8wo8vytpafln8);
		tester.register_test("File#pg23loxd7hva5ty7", test_pg23loxd7hva5ty7);
	}

	void register_read_exact_write_exact()
	{
		tester.register_test("File#0vj5qawe151dlab8", test_0vj5qawe151dlab8);
		tester.register_test("File#ip74fy4x4vq54ql3", test_ip74fy4x4vq54ql3);
		tester.register_test("File#h2ldps4weu4ia2s6", test_h2ldps4weu4ia2s6);
		tester.register_test("File#wjbtywqjl05sac7h", test_wjbtywqjl05sac7h);
	}

	void register_seek()
	{
		tester.register_test("File#1ywq7j1zp67guwip", test_1ywq7j1zp67guwip);
		tester.register_test("File#4r9qvyqhn1zlvzh8", test_4r9qvyqhn1zlvzh8);
		tester.register_test("File#g19x58du9tm8l39s", test_g19x58du9tm8l39s);
	}

	void register_skip()
	{
		tester.register_test("File#7flwx51n13c9137h", test_7flwx51n13c9137h);
		tester.register_test("File#s8ifik5wzumorn6t", test_s8ifik5wzumorn6t);
		tester.register_test("File#ykqf55ec945bin2j", test_ykqf55ec945bin2j);
	}

	void register_get_position()
	{
		tester.register_test("File#0gj54wvhksw88ae5", test_0gj54wvhksw88ae5);
		tester.register_test("File#7gwwf9pezbag4zt5", test_7gwwf9pezbag4zt5);
	}

	void register_set_position()
	{
		tester.register_test("File#ejj6s2bvcn5ba767", test_ejj6s2bvcn5ba767);
		tester.register_test("File#dqyslv8yuv2h8xpv", test_dqyslv8yuv2h8xpv);
		tester.register_test("File#wh4ro8o3yw0djyc6", test_wh4ro8o3yw0djyc6);
	}

	void register_get_size()
	{
		tester.register_test("File#q5bqlob75o008k21", test_q5bqlob75o008k21);
		tester.register_test("File#rt5sww69r9a7gj58", test_rt5sww69r9a7gj58);
	}

	void register_set_size()
	{
		tester.register_test("File#fuyi2tfqnsxirj3b", test_fuyi2tfqnsxirj3b);
		tester.register_test("File#dnpz1opo4nhy1irj", test_dnpz1opo4nhy1irj);
		tester.register_test("File#5pgsxx7g674b55nh", test_5pgsxx7g674b55nh);
	}

	void register_flush()
	{
		tester.register_test("File#4d4l6vgdguqfjarx", test_4d4l6vgdguqfjarx);
		tester.register_test("File#6xxbxfl2qvej8l07", test_6xxbxfl2qvej8l07);
	}
};

auto registrator = Registrator{};

} // namespace
