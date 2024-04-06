#include <algorithm>
#include <iterator>

#include "bstone_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// MemoryStream() noexcept
void test_ojtxcuz6rez2calp()
{
	const auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();

	tester.check(!is_open);
}

// MemoryStream(std::intptr_t, std::intptr_t)
void test_pkxovj4mzsohmn84()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	tester.check(is_open);
}

// MemoryStream(MemoryStream&&) noexcept
void test_teqabtkzn2m2k5ik()
{
	auto memory_stream_1 = bstone::MemoryStream{2, 2};
	const auto is_open_1 = memory_stream_1.is_open();
	auto memory_stream_2 = std::move(memory_stream_1);
	const auto is_open_2 = memory_stream_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// MemoryStream& operator=(MemoryStream&&) noexcept
void test_m0cri1qh6j1tm43t()
{
	auto memory_stream_1 = bstone::MemoryStream{2, 2};
	const auto is_open_1 = memory_stream_1.is_open();
	auto memory_stream_2 = std::move(memory_stream_1);
	const auto is_open_2 = memory_stream_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// ==========================================================================

// const std::uint8_t* get_data() const
void test_2sw1f8ldfqrhzioe()
{
	const auto get_const_stream = [](bstone::MemoryStream& stream) -> const bstone::MemoryStream&
	{
		return stream;
	};

	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	const auto data = get_const_stream(memory_stream).get_data();

	tester.check(is_open && data != nullptr);
}

// std::uint8_t* get_data()
void test_6dpdc2z906aj326e()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	const auto data = memory_stream.get_data();

	tester.check(is_open && data != nullptr);
}

// ==========================================================================

// void open(std::intptr_t, std::intptr_t)
void test_z0n6lao1idgwkvxf()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	memory_stream.open(2, 2);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// void close() noexcept
void test_a5fcwagrrawe30nh()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	memory_stream.open(2, 2);
	const auto is_open_2 = memory_stream.is_open();
	memory_stream.close();
	const auto is_open_3 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2 && !is_open_3);
}

// ==========================================================================

// is_open() const noexcept
void test_w4fbiti2lb0dhi8c()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	memory_stream.open(2, 2);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// std::intptr_t read(void*, std::intptr_t)
void test_rynbeatkwx2cg1tq()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
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

// ==========================================================================

// void read_exactly(void*, std::intptr_t)
void test_mqsd3narvervq1mj()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	auto storage = memory_stream.get_data();
	storage[0] = '#';
	memory_stream.set_size(1);
	memory_stream.read_exactly(&buffer_1, 1);
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && seek_result_2 && value_result_1);
}

// void read_exactly(void*, std::intptr_t)
// Failed.
void test_m1f9z70vrk1fsf4d()
{
	auto memory_stream = bstone::MemoryStream{1, 1};
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read_exactly(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// std::intptr_t write(const void*, std::intptr_t)
void test_k89hrws0i4vdcz7j()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
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
// New position is less or equal to the old size.
void test_c4xnnx60bh0el6c0()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();

	memory_stream.set_size(3);
	const auto old_size = memory_stream.get_size();
	const auto is_old_size_valid = old_size == 3;

	memory_stream.set_position(1);
	const auto old_position = memory_stream.get_position();
	const auto is_old_position_valid = old_position == 1;

	const auto ch = '\0';
	const auto written_size = memory_stream.write(&ch, 1);
	const auto is_written_size_valid = written_size == 1;

	const auto new_position = memory_stream.get_position();
	const auto is_new_position_valid = new_position == 2;

	const auto new_size = memory_stream.get_size();
	const auto is_new_size_valid = new_size == 3;

	const auto is_valid =
		is_open &&
		is_old_size_valid &&
		is_old_position_valid &&
		is_written_size_valid &&
		is_new_position_valid &&
		is_new_size_valid &&
		true;

	tester.check(is_valid);
}

// std::intptr_t write(const void*, std::intptr_t)
// New position is greater than the old size.
void test_ko6g2vzhmpoxkt3j()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();

	memory_stream.set_size(3);
	const auto old_size = memory_stream.get_size();
	const auto is_old_size_valid = old_size == 3;

	memory_stream.set_position(2);
	const auto old_position = memory_stream.get_position();
	const auto is_old_position_valid = old_position == 2;

	const char ch2[2] = {};
	const auto written_size = memory_stream.write(ch2, 2);
	const auto is_written_size_valid = written_size == 2;

	const auto new_position = memory_stream.get_position();
	const auto is_new_position_valid = new_position == 4;

	const auto new_size = memory_stream.get_size();
	const auto is_new_size_valid = new_size == 4;

	const auto is_valid =
		is_open &&
		is_old_size_valid &&
		is_old_position_valid &&
		is_written_size_valid &&
		is_new_position_valid &&
		is_new_size_valid &&
		true;

	tester.check(is_valid);
}

// ==========================================================================

// void write_exactly(const void*, std::intptr_t)
void test_c9md2uhmxgjpni9n()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const char buffer_1[4] = {'1', '2', '3', '4'};
	memory_stream.write_exactly(&buffer_1, 4);
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

// ==========================================================================

// std::int64_t seek(std::int64_t, StreamOrigin)
// Begin.
void test_xw9j98suf5ko60ol()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Current.
void test_yntiu3nk2y9hpose()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1100, bstone::StreamOrigin::begin) == 1100;
	const auto seek_result_2 = memory_stream.seek(-100, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// End.
void test_sy8ggddakci0on4o()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1100);
	const auto seek_result = memory_stream.seek(-100, bstone::StreamOrigin::end) == 1000;

	tester.check(is_open && seek_result);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// Negative new position.
void test_574sem81z8rm022v()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.seek(-100, bstone::StreamOrigin::end);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// std::int64_t seek(std::int64_t, StreamOrigin)
// New position out of range.
void test_394f5yz9rdgc4auk()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	memory_stream.seek(INTPTR_MAX, bstone::StreamOrigin::begin);

	try
	{
		memory_stream.seek(1, bstone::StreamOrigin::current);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// std::int64_t skip(std::int64_t)
void test_yurbec6ftn2g5avt()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.skip(-100) == 900;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// std::int64_t skip(std::int64_t)
// Negative new position.
void test_7bbudfbgrsrjptho()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
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
// New position out of range.
void test_9cfde0p326p1yu6d()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	memory_stream.skip(INTPTR_MAX);

	try
	{
		memory_stream.skip(1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// ==========================================================================

// std::int64_t get_position()
void test_onp8o681eu4edffa()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.get_position() == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// ==========================================================================

// void set_position(std::int64_t)
void test_bu2j1ikot7dc8l90()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_position(1000);
	const auto seek_result = memory_stream.seek(0, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result);
}

// void set_position(std::int64_t)
// Negative new position.
void test_pvh8jh1qlwf2w2yb()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
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

// void set_position(std::int64_t)
// New position out of range (32-bit app).
void test_f3wd8v7btmri5atw()
{
#if INTPTR_MAX == INT32_MAX
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_position(INT64_MAX);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
#endif
}

// ==========================================================================

// std::int64_t get_size()
void test_nsft61aab9k78e5h()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto size_result_1 = memory_stream.get_size() == 0;
	const char buffer = '*';
	const auto write_result_1 = memory_stream.write(&buffer, 1) == 1;
	memory_stream.set_size(1000);
	const auto size_result_2 = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result_1 && write_result_1 && size_result_2);
}

// ==========================================================================

// void set_size(std::int64_t)
void test_bzo85u09lyp12ghe()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1000);
	const auto size_result = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result);
}

// void set_size(std::int64_t)
// Out of range (32-bit app).
void test_tdpwy14xt5fneiaq()
{
#if INTPTR_MAX == INT32_MAX
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_size(INT64_MAX);
	}
	catch (...)
	{
		is_failed = true;
	}
	
	tester.check(is_open && is_failed);
#endif
}

// ==========================================================================

// void flush()
void test_rbd7mdepf2tm7kxi()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.flush();

	tester.check(is_open);
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
		register_read_exactly();
		register_write();
		register_write_exactly();
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
		tester.register_test("MemoryStream#ojtxcuz6rez2calp", test_ojtxcuz6rez2calp);
		tester.register_test("MemoryStream#pkxovj4mzsohmn84", test_pkxovj4mzsohmn84);
		tester.register_test("MemoryStream#teqabtkzn2m2k5ik", test_teqabtkzn2m2k5ik);
		tester.register_test("MemoryStream#m0cri1qh6j1tm43t", test_m0cri1qh6j1tm43t);
	}

	void register_get_data()
	{
		tester.register_test("MemoryStream#2sw1f8ldfqrhzioe", test_2sw1f8ldfqrhzioe);
		tester.register_test("MemoryStream#6dpdc2z906aj326e", test_6dpdc2z906aj326e);
	}

	void register_open()
	{
		tester.register_test("MemoryStream#z0n6lao1idgwkvxf", test_z0n6lao1idgwkvxf);
	}

	void register_close()
	{
		tester.register_test("MemoryStream#a5fcwagrrawe30nh", test_a5fcwagrrawe30nh);
	}

	void register_is_open()
	{
		tester.register_test("MemoryStream#w4fbiti2lb0dhi8c", test_w4fbiti2lb0dhi8c);
	}

	void register_read()
	{
		tester.register_test("MemoryStream#rynbeatkwx2cg1tq", test_rynbeatkwx2cg1tq);
	}

	void register_read_exactly()
	{
		tester.register_test("MemoryStream#mqsd3narvervq1mj", test_mqsd3narvervq1mj);
		tester.register_test("MemoryStream#m1f9z70vrk1fsf4d", test_m1f9z70vrk1fsf4d);
	}

	void register_write()
	{
		tester.register_test("MemoryStream#k89hrws0i4vdcz7j", test_k89hrws0i4vdcz7j);
		tester.register_test("MemoryStream#c4xnnx60bh0el6c0", test_c4xnnx60bh0el6c0);
		tester.register_test("MemoryStream#ko6g2vzhmpoxkt3j", test_ko6g2vzhmpoxkt3j);
	}

	void register_write_exactly()
	{
		tester.register_test("MemoryStream#c9md2uhmxgjpni9n", test_c9md2uhmxgjpni9n);
	}

	void register_seek()
	{
		tester.register_test("MemoryStream#xw9j98suf5ko60ol", test_xw9j98suf5ko60ol);
		tester.register_test("MemoryStream#yntiu3nk2y9hpose", test_yntiu3nk2y9hpose);
		tester.register_test("MemoryStream#sy8ggddakci0on4o", test_sy8ggddakci0on4o);
		tester.register_test("MemoryStream#574sem81z8rm022v", test_574sem81z8rm022v);
		tester.register_test("MemoryStream#394f5yz9rdgc4auk", test_394f5yz9rdgc4auk);
	}

	void register_skip()
	{
		tester.register_test("MemoryStream#yurbec6ftn2g5avt", test_yurbec6ftn2g5avt);
		tester.register_test("MemoryStream#7bbudfbgrsrjptho", test_7bbudfbgrsrjptho);
		tester.register_test("MemoryStream#9cfde0p326p1yu6d", test_9cfde0p326p1yu6d);
	}

	void register_get_position()
	{
		tester.register_test("MemoryStream#onp8o681eu4edffa", test_onp8o681eu4edffa);
	}

	void register_set_position()
	{
		tester.register_test("MemoryStream#bu2j1ikot7dc8l90", test_bu2j1ikot7dc8l90);
		tester.register_test("MemoryStream#pvh8jh1qlwf2w2yb", test_pvh8jh1qlwf2w2yb);
		tester.register_test("MemoryStream#f3wd8v7btmri5atw", test_f3wd8v7btmri5atw);
	}

	void register_get_size()
	{
		tester.register_test("MemoryStream#nsft61aab9k78e5h", test_nsft61aab9k78e5h);
	}

	void register_set_size()
	{
		tester.register_test("MemoryStream#bzo85u09lyp12ghe", test_bzo85u09lyp12ghe);
		tester.register_test("MemoryStream#tdpwy14xt5fneiaq", test_tdpwy14xt5fneiaq);
	}

	void register_flush()
	{
		tester.register_test("MemoryStream#rbd7mdepf2tm7kxi", test_rbd7mdepf2tm7kxi);
	}
};

auto registrator = Registrator{};

} // namespace
