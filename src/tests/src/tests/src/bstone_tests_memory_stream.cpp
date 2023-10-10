#include <algorithm>
#include <iterator>

#include "bstone_memory_stream.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

// MemoryStream()
void test_ojtxcuz6rez2calp()
{
	const auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();

	tester.check(!is_open);
}

// MemoryStream(IntP, IntP)
void test_pkxovj4mzsohmn84()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	tester.check(is_open);
}

// MemoryStream(MemoryStream&&)
void test_teqabtkzn2m2k5ik()
{
	auto memory_stream_1 = bstone::MemoryStream{2, 2};
	const auto is_open_1 = memory_stream_1.is_open();
	auto memory_stream_2 = std::move(memory_stream_1);
	const auto is_open_2 = memory_stream_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// MemoryStream& operator=(MemoryStream&&)
void test_m0cri1qh6j1tm43t()
{
	auto memory_stream_1 = bstone::MemoryStream{2, 2};
	const auto is_open_1 = memory_stream_1.is_open();
	auto memory_stream_2 = std::move(memory_stream_1);
	const auto is_open_2 = memory_stream_2.is_open();

	tester.check(is_open_1 && is_open_2);
}

// ==========================================================================

// const UInt8* get_data() const
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

// const UInt8* get_data() const
// Closed.
void test_74qfvktioldtauvt()
{
	const auto get_const_stream = [](bstone::MemoryStream& stream) -> const bstone::MemoryStream&
	{
		return stream;
	};

	auto memory_stream = bstone::MemoryStream{};
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

// UInt8* get_data()
void test_6dpdc2z906aj326e()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	const auto data = memory_stream.get_data();

	tester.check(is_open && data != nullptr);
}

// UInt8* get_data()
// Closed.
void test_ohkkpxeelzjwuqib()
{
	auto memory_stream = bstone::MemoryStream{};
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

// open(IntP, IntP)
void test_z0n6lao1idgwkvxf()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	memory_stream.open(2, 2);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// close()
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

// is_open()
void test_w4fbiti2lb0dhi8c()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open_1 = memory_stream.is_open();
	memory_stream.open(2, 2);
	const auto is_open_2 = memory_stream.is_open();
	tester.check(!is_open_1 && is_open_2);
}

// ==========================================================================

// IntP read(void*, IntP)
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

// IntP read(void*, IntP)
// Closed.
void test_nyy0dd83c9pj6ecc()
{
	auto memory_stream = bstone::MemoryStream{};
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

// void read_exact(void*, IntP)
void test_mqsd3narvervq1mj()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	char buffer_1;
	auto storage = memory_stream.get_data();
	storage[0] = '#';
	memory_stream.set_size(1);
	memory_stream.read_exact(&buffer_1, 1);
	const auto seek_result_2 = memory_stream.seek(0, bstone::StreamOrigin::current) == 1;
	const auto value_result_1 = buffer_1 == '#';

	tester.check(is_open && seek_result_2 && value_result_1);
}

// void read_exact(void*, IntP)
// Failed.
void test_m1f9z70vrk1fsf4d()
{
	auto memory_stream = bstone::MemoryStream{1, 1};
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// void read_exact(void*, IntP)
// Failed.
void test_4twl7qht0bbikxnn()
{
	auto memory_stream = bstone::MemoryStream{};
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read_exact(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_failed);
}

// ==========================================================================

// IntP write(const void*, IntP)
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

// IntP write(const void*, IntP)
// Closed.
void test_j0c9s05ovmwnydbc()
{
	auto memory_stream = bstone::MemoryStream{};
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

// void write_exact(const void*, IntP)
void test_c9md2uhmxgjpni9n()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
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

// IntP write(const void*, IntP)
// Closed.
void test_879y5xi6dsxh4p20()
{
	auto memory_stream = bstone::MemoryStream{};
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

// Int64 seek(Int64, StreamOrigin)
// Begin.
void test_xw9j98suf5ko60ol()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;

	tester.check(is_open && seek_result);
}

// Int64 seek(Int64, StreamOrigin)
// Current.
void test_yntiu3nk2y9hpose()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1100, bstone::StreamOrigin::begin) == 1100;
	const auto seek_result_2 = memory_stream.seek(-100, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// Int64 seek(Int64, StreamOrigin)
// End.
void test_sy8ggddakci0on4o()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1100);
	const auto seek_result = memory_stream.seek(-100, bstone::StreamOrigin::end) == 1000;

	tester.check(is_open && seek_result);
}

// Int64 seek(Int64, StreamOrigin)
// Fail new position.
void test_574sem81z8rm022v()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
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

// Int64 seek(Int64, StreamOrigin)
// Closed.
void test_kgyq86m1pumnqidm()
{
	auto memory_stream = bstone::MemoryStream{};
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

// Int64 skip(Int64)
void test_yurbec6ftn2g5avt()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.skip(-100) == 900;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// Int64 skip(Int64)
// Fail new position.
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

// Int64 skip(Int64)
// Closed.
void test_aal89hd2cksyi004()
{
	auto memory_stream = bstone::MemoryStream{};
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

// Int64 get_position()
void test_onp8o681eu4edffa()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	const auto seek_result_1 = memory_stream.seek(1000, bstone::StreamOrigin::begin) == 1000;
	const auto seek_result_2 = memory_stream.get_position() == 1000;

	tester.check(is_open && seek_result_1 && seek_result_2);
}

// Int64 get_position()
// Closed.
void test_wjjccz8dqrmb60t6()
{
	auto memory_stream = bstone::MemoryStream{};
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

// void set_position(Int64)
void test_bu2j1ikot7dc8l90()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_position(1000);
	const auto seek_result = memory_stream.seek(0, bstone::StreamOrigin::current) == 1000;

	tester.check(is_open && seek_result);
}

// void set_position(Int64)
// Fail new position.
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

// void set_position(Int64)
// Closed.
void test_i2dm7qori0hcqr4b()
{
	auto memory_stream = bstone::MemoryStream{};
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

// Int64 get_size() const
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

// Int64 get_size() const
// Closed.
void test_jtu27v4hubyx3s33()
{
	auto memory_stream = bstone::MemoryStream{};
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

// set_size(Int64)
void test_bzo85u09lyp12ghe()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.set_size(1000);
	const auto size_result = memory_stream.get_size() == 1000;

	tester.check(is_open && size_result);
}

// set_size(Int64)
// Closed.
void test_ujjzpavnpwcjztzj()
{
	auto memory_stream = bstone::MemoryStream{};
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
void test_rbd7mdepf2tm7kxi()
{
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	memory_stream.flush();

	tester.check(is_open);
}

// flush()
// Closed.
void test_rgokki5yxd53uy9g()
{
	auto memory_stream = bstone::MemoryStream{};
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
		tester.register_test("MemoryStream#ojtxcuz6rez2calp", test_ojtxcuz6rez2calp);
		tester.register_test("MemoryStream#pkxovj4mzsohmn84", test_pkxovj4mzsohmn84);
		tester.register_test("MemoryStream#teqabtkzn2m2k5ik", test_teqabtkzn2m2k5ik);
		tester.register_test("MemoryStream#m0cri1qh6j1tm43t", test_m0cri1qh6j1tm43t);
	}

	void register_get_data()
	{
		tester.register_test("MemoryStream#2sw1f8ldfqrhzioe", test_2sw1f8ldfqrhzioe);
		tester.register_test("MemoryStream#74qfvktioldtauvt", test_74qfvktioldtauvt);
		tester.register_test("MemoryStream#6dpdc2z906aj326e", test_6dpdc2z906aj326e);
		tester.register_test("MemoryStream#ohkkpxeelzjwuqib", test_ohkkpxeelzjwuqib);
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
		tester.register_test("MemoryStream#nyy0dd83c9pj6ecc", test_nyy0dd83c9pj6ecc);
	}

	void register_read_exact()
	{
		tester.register_test("MemoryStream#mqsd3narvervq1mj", test_mqsd3narvervq1mj);
		tester.register_test("MemoryStream#m1f9z70vrk1fsf4d", test_m1f9z70vrk1fsf4d);
		tester.register_test("MemoryStream#4twl7qht0bbikxnn", test_4twl7qht0bbikxnn);
	}

	void register_write()
	{
		tester.register_test("MemoryStream#k89hrws0i4vdcz7j", test_k89hrws0i4vdcz7j);
		tester.register_test("MemoryStream#kgyq86m1pumnqidm", test_j0c9s05ovmwnydbc);
	}

	void register_write_exact()
	{
		tester.register_test("MemoryStream#c9md2uhmxgjpni9n", test_c9md2uhmxgjpni9n);
		tester.register_test("MemoryStream#879y5xi6dsxh4p20", test_879y5xi6dsxh4p20);
	}

	void register_seek()
	{
		tester.register_test("MemoryStream#xw9j98suf5ko60ol", test_xw9j98suf5ko60ol);
		tester.register_test("MemoryStream#yntiu3nk2y9hpose", test_yntiu3nk2y9hpose);
		tester.register_test("MemoryStream#sy8ggddakci0on4o", test_sy8ggddakci0on4o);
		tester.register_test("MemoryStream#574sem81z8rm022v", test_574sem81z8rm022v);
		tester.register_test("MemoryStream#kgyq86m1pumnqidm", test_kgyq86m1pumnqidm);
	}

	void register_skip()
	{
		tester.register_test("MemoryStream#yurbec6ftn2g5avt", test_yurbec6ftn2g5avt);
		tester.register_test("MemoryStream#7bbudfbgrsrjptho", test_7bbudfbgrsrjptho);
		tester.register_test("MemoryStream#aal89hd2cksyi004", test_aal89hd2cksyi004);
	}

	void register_get_position()
	{
		tester.register_test("MemoryStream#onp8o681eu4edffa", test_onp8o681eu4edffa);
		tester.register_test("MemoryStream#wjjccz8dqrmb60t6", test_wjjccz8dqrmb60t6);
	}

	void register_set_position()
	{
		tester.register_test("MemoryStream#bu2j1ikot7dc8l90", test_bu2j1ikot7dc8l90);
		tester.register_test("MemoryStream#pvh8jh1qlwf2w2yb", test_pvh8jh1qlwf2w2yb);
		tester.register_test("MemoryStream#i2dm7qori0hcqr4b", test_i2dm7qori0hcqr4b);
	}

	void register_get_size()
	{
		tester.register_test("MemoryStream#nsft61aab9k78e5h", test_nsft61aab9k78e5h);
		tester.register_test("MemoryStream#jtu27v4hubyx3s33", test_jtu27v4hubyx3s33);
	}

	void register_set_size()
	{
		tester.register_test("MemoryStream#bzo85u09lyp12ghe", test_bzo85u09lyp12ghe);
		tester.register_test("MemoryStream#ujjzpavnpwcjztzj", test_ujjzpavnpwcjztzj);
	}

	void register_flush()
	{
		tester.register_test("MemoryStream#rbd7mdepf2tm7kxi", test_rbd7mdepf2tm7kxi);
		tester.register_test("MemoryStream#rgokki5yxd53uy9g", test_rgokki5yxd53uy9g);
	}
};

auto registrator = Registrator{};

} // namespace
