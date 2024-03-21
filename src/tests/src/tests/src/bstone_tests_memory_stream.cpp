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

// MemoryStream(std::intptr_t, std::intptr_t)
// Negative capacity.
void test_oxsseq12zq4gc0mh()
{
	auto is_open = false;
	auto is_failed = false;

	try
	{
		auto memory_stream = bstone::MemoryStream{-2, 2};
		is_open = memory_stream.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// MemoryStream(std::intptr_t, std::intptr_t)
// Negative chunk size.
void test_de36ayiiisb95y67()
{
	auto is_open = false;
	auto is_failed = false;

	try
	{
		auto memory_stream = bstone::MemoryStream{2, -2};
		is_open = memory_stream.is_open();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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

// const std::uint8_t* get_data() const
// Closed.
void test_facj7939pzkqzmpe()
{
	const auto get_const_stream = [](bstone::MemoryStream& stream) -> const bstone::MemoryStream&
	{
		return stream;
	};

	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		get_const_stream(memory_stream).get_data();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// std::uint8_t* get_data()
void test_6dpdc2z906aj326e()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	const auto data = memory_stream.get_data();

	tester.check(is_open && data != nullptr);
}

// std::uint8_t* get_data()
// Closed.
void test_n709wxeu17ht0j4c()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.get_data();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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

// void open(std::intptr_t, std::intptr_t)
// Negative capacity.
void test_fzf597nw6linsoxi()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.open(-2, 2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// void open(std::intptr_t, std::intptr_t)
// Negative chunk size.
void test_5p1ropw65rwl05pg()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.open(2, -2);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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

// std::intptr_t read(void*, std::intptr_t)
// Closed.
void test_bnjkrssn3bby8ek3()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// std::intptr_t read(void*, std::intptr_t)
// Null buffer.
void test_ehx0pjs8b3qti571()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.read(nullptr, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// std::intptr_t read(void*, std::intptr_t)
// Negative count.
void test_spaicu71aiwvirty()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read(&buffer, -1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
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

// void read_exactly(void*, std::intptr_t)
// Closed.
void test_38irjbcrxvzq2qas()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
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

	tester.check(!is_open && is_failed);
}

// void read_exactly(void*, std::intptr_t)
// Null buffer.
void test_sa1zhnmlqvwlsv7w()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.read_exactly(nullptr, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void read_exactly(void*, std::intptr_t)
// Negative count.
void test_h7c0llq3kh1my1bl()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		auto buffer = '\0';
		memory_stream.read_exactly(&buffer, -1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
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
// Closed.
void test_fixlxu7dixu74tt9()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		const auto buffer = '\0';
		memory_stream.write(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// std::intptr_t write(const void*, std::intptr_t)
// Null buffer.
void test_v7ss0d66zvp6u18a()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.write(nullptr, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// std::intptr_t write(const void*, std::intptr_t)
// Negative count.
void test_euqbk0qtfs5ogy4g()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		const auto buffer = '\0';
		memory_stream.write(&buffer, -1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
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

// void write_exactly(const void*, std::intptr_t)
// Closed.
void test_2c8dxrivzvnb9201()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		const auto buffer = '\0';
		memory_stream.write_exactly(&buffer, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// void write_exactly(const void*, std::intptr_t)
// Null buffer.
void test_wdnd1f8yao2rv83n()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.write_exactly(nullptr, 1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
}

// void write_exactly(const void*, std::intptr_t)
// Negative count.
void test_pxly3lq6nhn356x6()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		const auto buffer = '\0';
		memory_stream.write_exactly(&buffer, -1);
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
// Closed.
void test_a2m96f9w35u4oblr()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.seek(1000, bstone::StreamOrigin::begin);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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
// New position out of range (32-bit app).
void test_ngur9azed7ywerfu()
{
#if INTPTR_MAX == INT32_MAX
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.seek(INT64_MAX, bstone::StreamOrigin::begin);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
#endif
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
// Closed.
void test_1lsxi7q8s4wk0rv9()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.skip(1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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
// New position out of range (32-bit app).
void test_npvi1bgdnv9fjht2()
{
#if INTPTR_MAX == INT32_MAX
	auto memory_stream = bstone::MemoryStream{4, 1};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.skip(INT64_MAX);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
#endif
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

// std::int64_t get_position()
// Closed.
void test_14656sp8iypp47fc()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.get_position();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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
// Closed.
void test_tyd0se4j6dioe3y9()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_position(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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

// std::int64_t get_size()
// Closed.
void test_p3imcry1c30uerh4()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.get_size();
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
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
// Closed.
void test_vtfdh5a5m1m9di51()
{
	auto memory_stream = bstone::MemoryStream{};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_size(0);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(!is_open && is_failed);
}

// void set_size(std::int64_t)
// Negative size.
void test_crqda9voeeft5tsz()
{
	auto memory_stream = bstone::MemoryStream{2, 2};
	const auto is_open = memory_stream.is_open();
	auto is_failed = false;

	try
	{
		memory_stream.set_size(-1);
	}
	catch (...)
	{
		is_failed = true;
	}

	tester.check(is_open && is_failed);
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

// void flush()
// Closed.
void test_il18y7brprn57yr2()
{
	auto memory_stream = bstone::MemoryStream{};
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

	tester.check(!is_open && is_failed);
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
		tester.register_test("MemoryStream#oxsseq12zq4gc0mh", test_oxsseq12zq4gc0mh);
		tester.register_test("MemoryStream#de36ayiiisb95y67", test_de36ayiiisb95y67);
		tester.register_test("MemoryStream#teqabtkzn2m2k5ik", test_teqabtkzn2m2k5ik);
		tester.register_test("MemoryStream#m0cri1qh6j1tm43t", test_m0cri1qh6j1tm43t);
	}

	void register_get_data()
	{
		tester.register_test("MemoryStream#2sw1f8ldfqrhzioe", test_2sw1f8ldfqrhzioe);
		tester.register_test("MemoryStream#facj7939pzkqzmpe", test_facj7939pzkqzmpe);
		tester.register_test("MemoryStream#6dpdc2z906aj326e", test_6dpdc2z906aj326e);
		tester.register_test("MemoryStream#n709wxeu17ht0j4c", test_n709wxeu17ht0j4c);
	}

	void register_open()
	{
		tester.register_test("MemoryStream#z0n6lao1idgwkvxf", test_z0n6lao1idgwkvxf);
		tester.register_test("MemoryStream#fzf597nw6linsoxi", test_fzf597nw6linsoxi);
		tester.register_test("MemoryStream#5p1ropw65rwl05pg", test_5p1ropw65rwl05pg);
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
		tester.register_test("MemoryStream#bnjkrssn3bby8ek3", test_bnjkrssn3bby8ek3);
		tester.register_test("MemoryStream#ehx0pjs8b3qti571", test_ehx0pjs8b3qti571);
		tester.register_test("MemoryStream#spaicu71aiwvirty", test_spaicu71aiwvirty);
	}

	void register_read_exactly()
	{
		tester.register_test("MemoryStream#mqsd3narvervq1mj", test_mqsd3narvervq1mj);
		tester.register_test("MemoryStream#m1f9z70vrk1fsf4d", test_m1f9z70vrk1fsf4d);
		tester.register_test("MemoryStream#38irjbcrxvzq2qas", test_38irjbcrxvzq2qas);
		tester.register_test("MemoryStream#sa1zhnmlqvwlsv7w", test_sa1zhnmlqvwlsv7w);
		tester.register_test("MemoryStream#h7c0llq3kh1my1bl", test_h7c0llq3kh1my1bl);
	}

	void register_write()
	{
		tester.register_test("MemoryStream#k89hrws0i4vdcz7j", test_k89hrws0i4vdcz7j);
		tester.register_test("MemoryStream#fixlxu7dixu74tt9", test_fixlxu7dixu74tt9);
		tester.register_test("MemoryStream#v7ss0d66zvp6u18a", test_v7ss0d66zvp6u18a);
		tester.register_test("MemoryStream#euqbk0qtfs5ogy4g", test_euqbk0qtfs5ogy4g);
	}

	void register_write_exactly()
	{
		tester.register_test("MemoryStream#c9md2uhmxgjpni9n", test_c9md2uhmxgjpni9n);
		tester.register_test("MemoryStream#2c8dxrivzvnb9201", test_2c8dxrivzvnb9201);
		tester.register_test("MemoryStream#wdnd1f8yao2rv83n", test_wdnd1f8yao2rv83n);
		tester.register_test("MemoryStream#pxly3lq6nhn356x6", test_pxly3lq6nhn356x6);
	}

	void register_seek()
	{
		tester.register_test("MemoryStream#xw9j98suf5ko60ol", test_xw9j98suf5ko60ol);
		tester.register_test("MemoryStream#yntiu3nk2y9hpose", test_yntiu3nk2y9hpose);
		tester.register_test("MemoryStream#sy8ggddakci0on4o", test_sy8ggddakci0on4o);
		tester.register_test("MemoryStream#a2m96f9w35u4oblr", test_a2m96f9w35u4oblr);
		tester.register_test("MemoryStream#574sem81z8rm022v", test_574sem81z8rm022v);
		tester.register_test("MemoryStream#ngur9azed7ywerfu", test_ngur9azed7ywerfu);
		tester.register_test("MemoryStream#394f5yz9rdgc4auk", test_394f5yz9rdgc4auk);
	}

	void register_skip()
	{
		tester.register_test("MemoryStream#yurbec6ftn2g5avt", test_yurbec6ftn2g5avt);
		tester.register_test("MemoryStream#1lsxi7q8s4wk0rv9", test_1lsxi7q8s4wk0rv9);
		tester.register_test("MemoryStream#7bbudfbgrsrjptho", test_7bbudfbgrsrjptho);
		tester.register_test("MemoryStream#npvi1bgdnv9fjht2", test_npvi1bgdnv9fjht2);
		tester.register_test("MemoryStream#9cfde0p326p1yu6d", test_9cfde0p326p1yu6d);
	}

	void register_get_position()
	{
		tester.register_test("MemoryStream#onp8o681eu4edffa", test_onp8o681eu4edffa);
		tester.register_test("MemoryStream#14656sp8iypp47fc", test_14656sp8iypp47fc);
	}

	void register_set_position()
	{
		tester.register_test("MemoryStream#bu2j1ikot7dc8l90", test_bu2j1ikot7dc8l90);
		tester.register_test("MemoryStream#tyd0se4j6dioe3y9", test_tyd0se4j6dioe3y9);
		tester.register_test("MemoryStream#pvh8jh1qlwf2w2yb", test_pvh8jh1qlwf2w2yb);
		tester.register_test("MemoryStream#f3wd8v7btmri5atw", test_f3wd8v7btmri5atw);
	}

	void register_get_size()
	{
		tester.register_test("MemoryStream#nsft61aab9k78e5h", test_nsft61aab9k78e5h);
		tester.register_test("MemoryStream#p3imcry1c30uerh4", test_p3imcry1c30uerh4);
	}

	void register_set_size()
	{
		tester.register_test("MemoryStream#bzo85u09lyp12ghe", test_bzo85u09lyp12ghe);
		tester.register_test("MemoryStream#vtfdh5a5m1m9di51", test_vtfdh5a5m1m9di51);
		tester.register_test("MemoryStream#crqda9voeeft5tsz", test_crqda9voeeft5tsz);
	}

	void register_flush()
	{
		tester.register_test("MemoryStream#rbd7mdepf2tm7kxi", test_rbd7mdepf2tm7kxi);
		tester.register_test("MemoryStream#il18y7brprn57yr2", test_il18y7brprn57yr2);
	}
};

auto registrator = Registrator{};

} // namespace
