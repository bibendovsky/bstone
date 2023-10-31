#include <algorithm>
#include <iterator>

#include "bstone_static_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// StaticMemoryStream()
void test_6ds129w9dxsjomlc()
{
	const auto memory_stream = bstone::StaticMemoryStream{};
	const auto is_open = memory_stream.is_open();

	tester.check(!is_open);
}

// StaticMemoryStream(void*, std::intptr_t)
void test_cg874043ah1bqjbr()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	tester.check(is_open);
}

// StaticMemoryStream(const StaticMemoryStream&)
void test_f7khj63zk124btzp()
{
	char buffer[100] = {};
	auto memory_stream_1 = bstone::StaticMemoryStream{&buffer, 100};
	const auto is_open_1 = memory_stream_1.is_open();
	memory_stream_1.set_size(100);
	memory_stream_1.seek(1000, bstone::StreamOrigin::begin);
	const auto seek_result_1 = memory_stream_1.seek(0, bstone::StreamOrigin::current) == 1000;
	const auto size_result_1 = memory_stream_1.get_size() == 100;
	const auto data_result_1 = memory_stream_1.get_data() == reinterpret_cast<std::uint8_t*>(buffer);

	auto memory_stream_2 = memory_stream_1;
	const auto is_open_2 = memory_stream_2.is_open();
	const auto seek_result_2 = memory_stream_2.seek(0, bstone::StreamOrigin::current) == 1000;
	const auto size_result_2 = memory_stream_2.get_size() == 100;
	const auto data_result_2 = memory_stream_2.get_data() == reinterpret_cast<std::uint8_t*>(buffer);

	tester.check(
		is_open_1 &&
		seek_result_1 &&
		size_result_1 &&
		data_result_1 &&

		is_open_2 &&
		seek_result_2 &&
		size_result_2 &&
		data_result_2);
}

// StaticMemoryStream& operator=(const StaticMemoryStream&)
void test_00qcn6my0tb8k541()
{
	char buffer[100] = {};
	auto memory_stream_1 = bstone::StaticMemoryStream{&buffer, 100};
	const auto is_open_1 = memory_stream_1.is_open();
	memory_stream_1.set_size(100);
	memory_stream_1.seek(1000, bstone::StreamOrigin::begin);
	const auto seek_result_1 = memory_stream_1.seek(0, bstone::StreamOrigin::current) == 1000;
	const auto size_result_1 = memory_stream_1.get_size() == 100;
	const auto data_result_1 = memory_stream_1.get_data() == reinterpret_cast<std::uint8_t*>(buffer);

	auto memory_stream_2 = bstone::StaticMemoryStream{};
	memory_stream_2 = memory_stream_1;
	const auto is_open_2 = memory_stream_2.is_open();
	const auto seek_result_2 = memory_stream_2.seek(0, bstone::StreamOrigin::current) == 1000;
	const auto size_result_2 = memory_stream_2.get_size() == 100;
	const auto data_result_2 = memory_stream_2.get_data() == reinterpret_cast<std::uint8_t*>(buffer);

	tester.check(
		is_open_1 &&
		seek_result_1 &&
		size_result_1 &&
		data_result_1 &&

		is_open_2 &&
		seek_result_2 &&
		size_result_2 &&
		data_result_2);
}

// ==========================================================================

// const std::uint8_t* get_data() const
void test_t3eqak5thhnos9xe()
{
	const auto get_const_stream = [](bstone::StaticMemoryStream& stream) -> const bstone::StaticMemoryStream&
	{
		return stream;
	};

	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto data = get_const_stream(memory_stream).get_data();

	tester.check(is_open && data != nullptr);
}

// const std::uint8_t* get_data() const
// Closed.
void test_necdmrdwdrcpmw6t()
{
	const auto get_const_stream = [](bstone::StaticMemoryStream& stream) -> const bstone::StaticMemoryStream&
	{
		return stream;
	};

	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		get_const_stream(memory_stream).get_data();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// std::uint8_t* get_data()
void test_g5ff2gqbyeid3ezx()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto data = memory_stream.get_data();

	tester.check(is_open && data != nullptr);
}

// std::uint8_t* get_data()
// Closed.
void test_wrdrulzvlbr06hu2()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.get_data();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// open(std::intptr_t, std::intptr_t)
void test_50buk347p5mbwcdn()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// close()
void test_8j10siejav8rid6d()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	memory_stream.close();
	const auto is_open_3 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2 && !is_open_3);
}

// ==========================================================================

// is_open()
void test_2siufbq86mhyo1ua()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// std::intptr_t read(void*, std::intptr_t)
void test_1mzma6x2ck33dfws()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	const auto read_result_1 = memory_stream.read(&buffer_1, 1) == 0;
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 0;
	auto storage = memory_stream.get_data();
	storage[0] = '#';
	memory_stream.set_size(1);
	const auto read_result_2 = memory_stream.read(&buffer_1, 1) == 1;
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && read_result_1 && seek_result_1 && read_result_2 && seek_result_2 && value_result_1);
}

// std::intptr_t read(void*, std::intptr_t)
// Closed.
void test_eet30c1oijx76qdz()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	char buffer;
	auto is_failed = false;

	try
	{
		memory_stream.read(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// void read_exact(void*, std::intptr_t)
void test_u4a8sprkw3hjgtes()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	const auto read_result_1 = memory_stream.read(&buffer_1, 1) == 0;
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 0;
	auto storage = memory_stream.get_data();
	storage[0] = '#';
	memory_stream.set_size(1);
	memory_stream.read_exact(&buffer_1, 1);
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && read_result_1 && seek_result_1 && seek_result_2 && value_result_1);
}

// void read_exact(void*, std::intptr_t)
// Failed.
void test_2yv6r8s3ry3upqxm()
{
	char buffer[2] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1);

	auto is_failed = false;

	try
	{
		char buffer_2[2];
		memory_stream.read_exact(buffer_2, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void read_exact(void*, std::intptr_t)
// Closed.
void test_lvdubbybe0spue34()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	char buffer;
	auto is_failed = false;

	try
	{
		memory_stream.read(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::intptr_t write(const void*, std::intptr_t)
void test_14mn4jrmdfhbw8dd()
{
	char buffer[4] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 4};
	const auto is_open = memory_stream.is_open();
	const char buffer_1[4] = {'1', '2', '3', '4'};
	const auto write_result_1 = memory_stream.write(&buffer_1, 4) == 4;
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 4;

	const auto storage_result =
		buffer_1[0] == '1' && buffer_1[1] == '2' && buffer_1[2] == '3' && buffer_1[3] == '4';

	char buffer_2[4];
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::begin) == 0;
	const auto read_result_2 = memory_stream.read(&buffer_2, 4) == 4;

	const auto value_result_1 =
		buffer_2[0] == buffer_1[0] &&
		buffer_2[1] == buffer_1[1] &&
		buffer_2[2] == buffer_1[2] &&
		buffer_2[3] == buffer_1[3];

	tester.check(
		is_open &&
		write_result_1 &&
		seek_result_1 &&
		storage_result &&
		seek_result_2 &&
		read_result_2 &&
		value_result_1);
}

// std::intptr_t write(const void*, std::intptr_t)
// Closed.
void test_59vfyvgpyhmuqh4a()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	char buffer = '\0';
	auto is_failed = false;

	try
	{
		memory_stream.write(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// void write_exact(const void*, std::intptr_t)
void test_bl9qhqrhs3w0hn33()
{
	char buffer[4] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 4};
	const auto is_open = memory_stream.is_open();
	const char buffer_1[4] = {'1', '2', '3', '4'};
	memory_stream.write_exact(&buffer_1, 4);
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 4;

	const auto storage_result =
		buffer_1[0] == '1' && buffer_1[1] == '2' && buffer_1[2] == '3' && buffer_1[3] == '4';

	char buffer_2[4];
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::begin) == 0;
	const auto read_result_2 = memory_stream.read(&buffer_2, 4) == 4;

	const auto value_result_1 =
		buffer_2[0] == buffer_1[0] &&
		buffer_2[1] == buffer_1[1] &&
		buffer_2[2] == buffer_1[2] &&
		buffer_2[3] == buffer_1[3];

	tester.check(
		is_open &&
		seek_result_1 &&
		storage_result &&
		seek_result_2 &&
		read_result_2 &&
		value_result_1);
}

// void write_exact(const void*, std::intptr_t)
// Failed.
void test_d0jinhmh1kfkxt95()
{
	char buffer[2] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		const char buffer_1[2] = {};
		memory_stream.write_exact(&buffer_1, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void write_exact(const void*, std::intptr_t)
// Closed.
void test_yyalh5kuft5fimma()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		char buffer = '\0';
		memory_stream.write_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::int64_t seek(std::int64_t, StreamOrigin)
// Begin.
void test_qiihjf8xgy3e6ead()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Current.
void test_p7p8fx1a861mqtfg()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1100, bstone::StreamOrigin::begin) == 1100;
	const auto seek_result_2 = memory_stream.seek(-100, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// End.
void test_6g653ulwicud0fnz()
{
	char buffer[1100] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1100};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1100);
	const auto seek_result = memory_stream.seek(-100, bstone::StreamOrigin::end) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Fail new position.
void test_cv5v7g2aqhnd6c7c()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;
	auto seek_result = false;

	try
	{
		seek_result = memory_stream.seek(-100, bstone::StreamOrigin::end) == -100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed && !seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Closed.
void test_hnb88o1do1zil9a7()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.seek(0, bstone::StreamOrigin::begin);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::int64_t skip(std::int64_t)
void test_ld3x5fvllannxapc()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.skip(1000) == 1000;
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t skip(std::int64_t)
// Fail new position.
void test_56gj0c3jb9k386u9()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;
	auto seek_result = false;

	try
	{
		seek_result = memory_stream.skip(-100) == -100;
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed && !seek_result);
}

// std::int64_t skip(std::int64_t)
// Closed.
void test_ct2vcyf0vuumzuml()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.skip(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::int64_t get_position()
void test_8nyr5ardlal737iz()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.get_position() == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t get_position()
// Closed.
void test_11lh7i6ccqb6cgw3()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.get_position();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// void set_position()
void test_lnen4qjm4uls7hzn()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_position(1000);
	const auto seek_result = memory_stream.seek(0, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result);
}

// void set_position()
// Fail new position.
void test_ai6l5wadrz6ry50b()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_position(-100);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void set_position()
// Closed.
void test_4xn51ys09fwpw4n2()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.set_position(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::int64_t get_size() const
void test_ue3pgnp5r744w081()
{
	char buffer[1000] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1000};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1000);
	const auto size_result = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result);
}

// std::int64_t get_size() const
// Closed.
void test_ksyf1b712t8xomx6()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.get_size();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// set_size(std::int64_t)
void test_a9babwqz5iahlltw()
{
	char buffer[1000] = {};
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1000};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1000);
	const auto size_result_1 = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result_1);
}

// set_size(std::int64_t)
// Closed.
void test_90sbsesx8lmlofyx()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.set_size(1000);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// flush()
void test_889yv25c2jyd208c()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.flush();

	tester.check(is_open);
}

// flush()
// Closed.
void test_q98coo456jrrou9j()
{
	auto memory_stream = bstone::StaticMemoryStream{};
	auto is_failed = false;

	try
	{
		memory_stream.flush();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_memory_stream();
		register_get_data();
		register_open();
		register_close();
		register_is_open();
		register_read();
		register_read_exact();
		register_write();
		register_write_exact();
		register_seek();
		register_skip();
		register_get_position();
		register_set_position();
		register_get_size();
		register_set_size();
		register_flush();
	}

private:
	void register_memory_stream()
	{
		tester.register_test("StaticMemoryStream#6ds129w9dxsjomlc", test_6ds129w9dxsjomlc);
		tester.register_test("StaticMemoryStream#cg874043ah1bqjbr", test_cg874043ah1bqjbr);
		tester.register_test("StaticMemoryStream#f7khj63zk124btzp", test_f7khj63zk124btzp);
		tester.register_test("StaticMemoryStream#00qcn6my0tb8k541", test_00qcn6my0tb8k541);
	}

	void register_get_data()
	{
		tester.register_test("StaticMemoryStream#t3eqak5thhnos9xe", test_t3eqak5thhnos9xe);
		tester.register_test("StaticMemoryStream#necdmrdwdrcpmw6t", test_necdmrdwdrcpmw6t);
		tester.register_test("StaticMemoryStream#g5ff2gqbyeid3ezx", test_g5ff2gqbyeid3ezx);
		tester.register_test("StaticMemoryStream#wrdrulzvlbr06hu2", test_wrdrulzvlbr06hu2);
	}

	void register_open()
	{
		tester.register_test("StaticMemoryStream#50buk347p5mbwcdn", test_50buk347p5mbwcdn);
	}

	void register_close()
	{
		tester.register_test("StaticMemoryStream#8j10siejav8rid6d", test_8j10siejav8rid6d);
	}

	void register_is_open()
	{
		tester.register_test("StaticMemoryStream#2siufbq86mhyo1ua", test_2siufbq86mhyo1ua);
	}

	void register_read()
	{
		tester.register_test("StaticMemoryStream#1mzma6x2ck33dfws", test_1mzma6x2ck33dfws);
		tester.register_test("StaticMemoryStream#eet30c1oijx76qdz", test_eet30c1oijx76qdz);
	}

	void register_read_exact()
	{
		tester.register_test("StaticMemoryStream#u4a8sprkw3hjgtes", test_u4a8sprkw3hjgtes);
		tester.register_test("StaticMemoryStream#2yv6r8s3ry3upqxm", test_2yv6r8s3ry3upqxm);
		tester.register_test("StaticMemoryStream#lvdubbybe0spue34", test_lvdubbybe0spue34);
	}

	void register_write()
	{
		tester.register_test("StaticMemoryStream#14mn4jrmdfhbw8dd", test_14mn4jrmdfhbw8dd);
		tester.register_test("StaticMemoryStream#59vfyvgpyhmuqh4a", test_59vfyvgpyhmuqh4a);
	}

	void register_write_exact()
	{
		tester.register_test("StaticMemoryStream#bl9qhqrhs3w0hn33", test_bl9qhqrhs3w0hn33);
		tester.register_test("StaticMemoryStream#d0jinhmh1kfkxt95", test_d0jinhmh1kfkxt95);
		tester.register_test("StaticMemoryStream#yyalh5kuft5fimma", test_yyalh5kuft5fimma);
	}

	void register_seek()
	{
		tester.register_test("StaticMemoryStream#qiihjf8xgy3e6ead", test_qiihjf8xgy3e6ead);
		tester.register_test("StaticMemoryStream#p7p8fx1a861mqtfg", test_p7p8fx1a861mqtfg);
		tester.register_test("StaticMemoryStream#6g653ulwicud0fnz", test_6g653ulwicud0fnz);
		tester.register_test("StaticMemoryStream#cv5v7g2aqhnd6c7c", test_cv5v7g2aqhnd6c7c);
		tester.register_test("StaticMemoryStream#hnb88o1do1zil9a7", test_hnb88o1do1zil9a7);
	}

	void register_skip()
	{
		tester.register_test("StaticMemoryStream#ld3x5fvllannxapc", test_ld3x5fvllannxapc);
		tester.register_test("StaticMemoryStream#56gj0c3jb9k386u9", test_56gj0c3jb9k386u9);
		tester.register_test("StaticMemoryStream#ct2vcyf0vuumzuml", test_ct2vcyf0vuumzuml);
	}

	void register_get_position()
	{
		tester.register_test("StaticMemoryStream#8nyr5ardlal737iz", test_8nyr5ardlal737iz);
		tester.register_test("StaticMemoryStream#11lh7i6ccqb6cgw3", test_11lh7i6ccqb6cgw3);
	}

	void register_set_position()
	{
		tester.register_test("StaticMemoryStream#lnen4qjm4uls7hzn", test_lnen4qjm4uls7hzn);
		tester.register_test("StaticMemoryStream#ai6l5wadrz6ry50b", test_ai6l5wadrz6ry50b);
		tester.register_test("StaticMemoryStream#4xn51ys09fwpw4n2", test_4xn51ys09fwpw4n2);
	}

	void register_get_size()
	{
		tester.register_test("StaticMemoryStream#ue3pgnp5r744w081", test_ue3pgnp5r744w081);
		tester.register_test("StaticMemoryStream#ksyf1b712t8xomx6", test_ksyf1b712t8xomx6);
	}

	void register_set_size()
	{
		tester.register_test("StaticMemoryStream#a9babwqz5iahlltw", test_a9babwqz5iahlltw);
		tester.register_test("StaticMemoryStream#90sbsesx8lmlofyx", test_90sbsesx8lmlofyx);
	}

	void register_flush()
	{
		tester.register_test("StaticMemoryStream#889yv25c2jyd208c", test_889yv25c2jyd208c);
		tester.register_test("StaticMemoryStream#q98coo456jrrou9j", test_q98coo456jrrou9j);
	}
};

auto registrator = Registrator{};

} // namespace
