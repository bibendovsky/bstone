#include <algorithm>
#include <iterator>

#include "bstone_static_ro_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// StaticRoMemoryStream() noexcept
void test_wq3fi41exm6zb6x3()
{
	const auto memory_stream = bstone::StaticRoMemoryStream{};
	const auto is_open = memory_stream.is_open();

	tester.check(!is_open);
}

// StaticRoMemoryStream(const void*, std::intptr_t) noexcept
void test_zj0inl335im315gs()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	tester.check(is_open);
}

// StaticRoMemoryStream(const StaticRoMemoryStream&) noexcept
void test_poherfnr0k8g1yeq()
{
	char buffer[100] = {};
	auto memory_stream_1 = bstone::StaticRoMemoryStream{&buffer, 100};
	const auto is_open_1 = memory_stream_1.is_open();
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

// StaticRoMemoryStream& operator=(const StaticRoMemoryStream&) noexcept
void test_u4n0mwpbhe0zh6dx()
{
	char buffer[100] = {};
	auto memory_stream_1 = bstone::StaticRoMemoryStream{&buffer, 100};
	const auto is_open_1 = memory_stream_1.is_open();
	memory_stream_1.seek(1000, bstone::StreamOrigin::begin);
	const auto seek_result_1 = memory_stream_1.seek(0, bstone::StreamOrigin::current) == 1000;
	const auto size_result_1 = memory_stream_1.get_size() == 100;
	const auto data_result_1 = memory_stream_1.get_data() == reinterpret_cast<std::uint8_t*>(buffer);

	auto memory_stream_2 = bstone::StaticRoMemoryStream{};
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

// const std::uint8_t* get_data() const noexcept
void test_qp9hhuxrpa93ofth()
{
	const auto get_const_stream = [](bstone::StaticRoMemoryStream& stream) -> const bstone::StaticRoMemoryStream&
	{
		return stream;
	};

	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto data = get_const_stream(memory_stream).get_data();

	tester.check(is_open && data == reinterpret_cast<const std::uint8_t*>(&buffer));
}

// const std::uint8_t* get_data() noexcept
void test_8kl1fz05db5sjl0c()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto data = memory_stream.get_data();

	tester.check(is_open && data == reinterpret_cast<const std::uint8_t*>(&buffer));
}

// ==========================================================================

// void open(const void*, std::intptr_t) noexcept
void test_ois2twhosp5eebex()
{
	auto memory_stream = bstone::StaticRoMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	const auto data = memory_stream.get_data();
	tester.check(!is_open_1 && is_open_2 && data == reinterpret_cast<const std::uint8_t*>(&buffer));
}

// ==========================================================================

// close() noexcept
void test_rief2dbpcv4xllsa()
{
	auto memory_stream = bstone::StaticRoMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	memory_stream.close();
	const auto is_open_3 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2 && !is_open_3);
}

// ==========================================================================

// bool is_open() const noexcept
void test_j7aq16ovijz9txgq()
{
	auto memory_stream = bstone::StaticRoMemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	char buffer = '\0';
	memory_stream.open(&buffer, 1);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// std::intptr_t read(void*, std::intptr_t)
void test_znfb1wjrycpg1hde()
{
	char buffer = '#';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	const auto read_result_1 = memory_stream.read(&buffer_1, 1) == 1;
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && read_result_1 && seek_result_1 && value_result_1);
}

// ==========================================================================

// void read_exact(void*, std::intptr_t)
void test_u4a8sprkw3hjgtes()
{
	char buffer = '#';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	memory_stream.read_exact(&buffer_1, 1);
	const auto seek_result_1 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && seek_result_1 && value_result_1);
}

// void read_exact(void*, std::intptr_t)
// Failed.
void test_2yv6r8s3ry3upqxm()
{
	char buffer = '#';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		char buffer_1[2] = {};
		memory_stream.read_exact(buffer_1, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// std::intptr_t write(const void*, std::intptr_t)
void test_3kzrr0bdk80gaue9()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.write(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// void write_exact(const void*, std::intptr_t)
void test_bl9qhqrhs3w0hn33()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.write_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// std::int64_t seek(std::int64_t, StreamOrigin)
// Begin.
void test_qpyvnxguh5wkoa1x()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Current.
void test_spzjjs24xs24yznc()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1100, bstone::StreamOrigin::begin) == 1100;
	const auto seek_result_2 = memory_stream.seek(-100, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// End.
void test_v74l7pqvo62lberx()
{
	char buffer[1100] = {};
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1100};
	const auto is_open = memory_stream.is_open();
	const auto seek_result = memory_stream.seek(-100, bstone::StreamOrigin::end) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Fail new position.
void test_zwk4vv1td5vw97q5()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
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

// ==========================================================================

// std::int64_t skip(std::int64_t)
void test_ld3x5fvllannxapc()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
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
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.skip(-100);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// std::int64_t get_position()
void test_8nyr5ardlal737iz()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.get_position() == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// ==========================================================================

// void set_position(std::int64_t)
void test_lnen4qjm4uls7hzn()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_position(1000);
	const auto seek_result = memory_stream.seek(0, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result);
}

// void set_position(std::int64_t)
// Fail new position.
void test_ai6l5wadrz6ry50b()
{
	char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
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

// ==========================================================================

// std::int64_t get_size()
void test_rcoycuoy6he01g9w()
{
	char buffer[1000] = {};
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1000};
	const auto is_open = memory_stream.is_open();
	const auto size_result = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result);
}

// ==========================================================================

// void set_size(std::int64_t)
void test_kr91vyb16y36bst7()
{
	const char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_size(1000);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// void flush()
void test_fo54zm1ngvaey6zi()
{
	const char buffer = '\0';
	auto memory_stream = bstone::StaticRoMemoryStream{&buffer, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.flush();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
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
		tester.register_test("StaticRoMemoryStream#wq3fi41exm6zb6x3", test_wq3fi41exm6zb6x3);
		tester.register_test("StaticRoMemoryStream#zj0inl335im315gs", test_zj0inl335im315gs);
		tester.register_test("StaticRoMemoryStream#poherfnr0k8g1yeq", test_poherfnr0k8g1yeq);
		tester.register_test("StaticRoMemoryStream#u4n0mwpbhe0zh6dx", test_u4n0mwpbhe0zh6dx);
	}

	void register_get_data()
	{
		tester.register_test("StaticRoMemoryStream#qp9hhuxrpa93ofth", test_qp9hhuxrpa93ofth);
		tester.register_test("StaticRoMemoryStream#8kl1fz05db5sjl0c", test_8kl1fz05db5sjl0c);
	}

	void register_open()
	{
		tester.register_test("StaticRoMemoryStream#ois2twhosp5eebex", test_ois2twhosp5eebex);
	}

	void register_close()
	{
		tester.register_test("StaticRoMemoryStream#rief2dbpcv4xllsa", test_rief2dbpcv4xllsa);
	}

	void register_is_open()
	{
		tester.register_test("StaticRoMemoryStream#j7aq16ovijz9txgq", test_j7aq16ovijz9txgq);
	}

	void register_read()
	{
		tester.register_test("StaticRoMemoryStream#znfb1wjrycpg1hde", test_znfb1wjrycpg1hde);
	}

	void register_read_exact()
	{
		tester.register_test("StaticRoMemoryStream#u4a8sprkw3hjgtes", test_u4a8sprkw3hjgtes);
		tester.register_test("StaticRoMemoryStream#2yv6r8s3ry3upqxm", test_2yv6r8s3ry3upqxm);
	}

	void register_write()
	{
		tester.register_test("StaticRoMemoryStream#3kzrr0bdk80gaue9", test_3kzrr0bdk80gaue9);
	}

	void register_write_exact()
	{
		tester.register_test("StaticRoMemoryStream#bl9qhqrhs3w0hn33", test_bl9qhqrhs3w0hn33);
	}

	void register_seek()
	{
		tester.register_test("StaticRoMemoryStream#qpyvnxguh5wkoa1x", test_qpyvnxguh5wkoa1x);
		tester.register_test("StaticRoMemoryStream#spzjjs24xs24yznc", test_spzjjs24xs24yznc);
		tester.register_test("StaticRoMemoryStream#v74l7pqvo62lberx", test_v74l7pqvo62lberx);
		tester.register_test("StaticRoMemoryStream#zwk4vv1td5vw97q5", test_zwk4vv1td5vw97q5);
	}

	void register_skip()
	{
		tester.register_test("StaticRoMemoryStream#ld3x5fvllannxapc", test_ld3x5fvllannxapc);
		tester.register_test("StaticRoMemoryStream#56gj0c3jb9k386u9", test_56gj0c3jb9k386u9);
	}

	void register_get_position()
	{
		tester.register_test("StaticRoMemoryStream#8nyr5ardlal737iz", test_8nyr5ardlal737iz);
	}

	void register_set_position()
	{
		tester.register_test("StaticRoMemoryStream#lnen4qjm4uls7hzn", test_lnen4qjm4uls7hzn);
		tester.register_test("StaticRoMemoryStream#ai6l5wadrz6ry50b", test_ai6l5wadrz6ry50b);
	}

	void register_get_size()
	{
		tester.register_test("StaticRoMemoryStream#rcoycuoy6he01g9w", test_rcoycuoy6he01g9w);
	}

	void register_set_size()
	{
		tester.register_test("StaticRoMemoryStream#kr91vyb16y36bst7", test_kr91vyb16y36bst7);
	}

	void register_flush()
	{
		tester.register_test("StaticRoMemoryStream#fo54zm1ngvaey6zi", test_fo54zm1ngvaey6zi);
	}
};

auto registrator = Registrator{};

} // namespace
