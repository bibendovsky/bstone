#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include "bstone_file.h"
#include "bstone_tester.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

const char* test_data_file_name = "test.data";
const char* existing_dir = "data";

// ==========================================================================

void remove_test_data_file()
{
	if (::remove(test_data_file_name))
	{
		throw std::runtime_error("Failed to remove test data file.");
	}
}

// ==========================================================================

void create_test_data_file()
{
	std::filebuf file;
	file.open(test_data_file_name, std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);

	if (!file.is_open())
	{
		throw std::runtime_error("Failed to create a test file.");
	}

	if (file.sputn("1234567890", 10) != 10)
	{
		throw std::runtime_error("Failed to write into test file.");
	}
}

// ==========================================================================
// File()
void test_ll78mzqsgafv1l26()
{
	bstone::File file;
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, read-only access.
void test_lq63at77az23n5si()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) < 0);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, write-only access.
void test_9kgqtp0jxn88o6sk()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_write);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, read-write access ("file_flags_read | file_flags_write").
void test_ss1k6dv22myc6wd8()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_write);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, read-write access ("file_flags_read_write").
void test_3w69uf8jer32pck6()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, create.
void test_aagzqydiws92d9p5()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_create);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, create and truncate.
void test_vxvnsqjp6plptr84()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Existing file, read-write access, create and truncate.
void test_w24wl6avxke60bwi()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// New file, create and truncate.
void test_5v1hdbekrnfub8ax()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// New file, read-write access, create and truncate.
void test_fq3iip5r1ike73dz()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Shared lock.
void test_w7b4aj4c9c7s6h7d()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared);
	tester.check(file_2.is_open());
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Exclusive lock.
void test_s5lnjx4gwgom7gmx()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	bstone::File file_2(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared);
	tester.check(!file_2.is_open());

	bstone::File file_3(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive);
	tester.check(!file_3.is_open());
// } POSIX-lock fails here.
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Empty path.
void test_95kkrfvi96b3zx8h()
{
	bstone::File file("", bstone::file_flags_read);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Missing file.
void test_x5qtvddnlo5tq9g1()
{
	bstone::File file(" ", bstone::file_flags_read);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Directory.
void test_bkfaq0pwod1b8zl2()
{
	bstone::File file(existing_dir, bstone::file_flags_read);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// No flags.
void test_0ab34jqqd4c340ll()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags)
// Surrogate code point.
void test_hpu1qzkaslmnmmof()
{
	bstone::File file("\xED\xA0\x80", bstone::file_flags_read);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-only access.
void test_us9pymgvin6ju1dg()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_read, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) < 0);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, write-only access.
void test_1itv9h1lsc0q8tt9()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_write, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access ("file_flags_read | file_flags_write").
void test_l3mosoqk31ffcjmp()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_write, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access ("file_flags_read_write").
void test_hs0nf4joyxfjke1t()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_read_write, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, create.
void test_9k3guxydb4m5wxx9()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_create, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, create and truncate.
void test_awwos5is0tdxp96w()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access, create and truncate.
void test_d3ogzymfmixkf15w()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// New file, create and truncate.
void test_thncsyg6wx4vte7x()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// New file, read access-write, create and truncate.
void test_pf40iv19vmld85xk()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Shared lock.
void test_djm5h61o0zt02c0k()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_2.is_open());
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Exclusive lock.
void test_gvixklv93pr1il3k()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code;

	buffer = 0;
	error_code = bstone::ec_file_none;
	bstone::File file_1(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive, error_code);
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	error_code = bstone::ec_file_none;
	bstone::File file_2(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file_2.is_open());

	error_code = bstone::ec_file_none;
	bstone::File file_3(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file_3.is_open());
// } POSIX-lock fails here.
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Empty path.
void test_a3bey5765ba9n5bc()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file("", bstone::file_flags_read, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Missing file.
void test_z7633fj9k8qtu7q7()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(" ", bstone::file_flags_read, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Directory.
void test_zkh3gf4znr40prib()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(existing_dir, bstone::file_flags_read, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// No flags.
void test_nio2oxxl2ycncdqr()
{
	create_test_data_file();
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file(test_data_file_name, bstone::file_flags_none, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// File(const char* path, FileFlags flags, FileErrorCode& error_code)
// Surrogate code point.
void test_1ml8olb35le5z411()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file("\xED\xA0\x80", bstone::file_flags_read, error_code);
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// bool is_open() const
void test_vm2wofyyq3lqmkif()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, read-only access.
void test_56fvardg84bl5zgr()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) < 0);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, write-only access.
void test_ybzz4gqodkco7a6e()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_write));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, read-write access ("file_flags_read | file_flags_write").
void test_zebwonr8ctsvrqs3()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_write));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, read-write access ("file_flags_read_write").
void test_gw6bveqqkje0t3co()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read_write));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, create.
void test_mwa9u1fbciegp6qw()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, create and truncate.
void test_s1kv571dol804hf3()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Existing file, read-write access, create and truncate.
void test_9eu656jx4efid17r()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// New file, create and truncate.
void test_2asori0d5innnejt()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// New file, read-write access, create and truncate.
void test_1gm81cu6jrgiz39y()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate));
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Shared lock.
void test_q1hgucxoq4ezusqi()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared));
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2;
	tester.check(file_2.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared));
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Exclusive lock.
void test_6stnrof91e93da36()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive));
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	bstone::File file_2;
	tester.check(!file_2.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared));
	tester.check(!file_2.is_open());

	bstone::File file_3;
	tester.check(!file_3.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive));
	tester.check(!file_3.is_open());
// } POSIX-lock fails here.
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Empty path.
void test_oaernzfasw8uxg3q()
{
	bstone::File file;
	tester.check(!file.open("", bstone::file_flags_read));
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Missing file.
void test_g299zwklmkjkk0xs()
{
	bstone::File file;
	tester.check(!file.open(" ", bstone::file_flags_read));
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Directory.
void test_o8097vrfgqon6uxt()
{
	bstone::File file;
	tester.check(!file.open(existing_dir, bstone::file_flags_read));
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// No flags.
void test_q3uux7jcmjtbfd6h()
{
	create_test_data_file();
	bstone::File file;
	tester.check(!file.open(test_data_file_name, bstone::file_flags_none));
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags)
// Surrogate code point.
void test_vz681wc4jnviony9()
{
	bstone::File file;
	tester.check(!file.open("\xED\xA0\x80", bstone::file_flags_read));
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-only access.
void test_sersynmko0no0w1k()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) < 0);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, write-only access.
void test_dq3pew0cd511rp3z()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_write, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access ("file_flags_read | file_flags_write").
void test_g7rs30gvw6ilzopv()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_write, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access ("file_flags_read_write").
void test_8sws6nqpqn9yr92m()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read_write, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 1);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, create.
void test_6zqgkqjbol5grihn()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, create and truncate.
void test_h01rlfpjkgsxwnst()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Existing file, read-write access, create and truncate.
void test_3nrw1jq80pxkl6pf()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// New file, create and truncate.
void test_z6xf1bjtrvt5srfd()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_create | bstone::file_flags_truncate, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) < 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// New file, read-write access, create and truncate.
void test_gzf3t7kux7352cwj()
{
	remove_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(file.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_create | bstone::file_flags_truncate, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file.is_open());
	tester.check(file.read(&buffer, 1) == 0);
	tester.check(file.write(&buffer, 1) == 1);
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Shared lock.
void test_8k4vayfahmm5qlho()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code = bstone::ec_file_none;

	buffer = 0;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2;
	tester.check(file_2.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Exclusive lock.
void test_p9r5hhb20dtig832()
{
	create_test_data_file();
	unsigned char buffer;
	bstone::FileErrorCode error_code;

	buffer = 0;
	error_code = bstone::ec_file_none;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive, error_code));
	tester.check(error_code == bstone::ec_file_none);
	tester.check(file_1.is_open());
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	error_code = bstone::ec_file_none;
	bstone::File file_2;
	tester.check(!file_2.open(test_data_file_name, bstone::file_flags_write | bstone::file_flags_shared, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file_2.is_open());

	error_code = bstone::ec_file_none;
	bstone::File file_3;
	tester.check(!file_3.open(test_data_file_name, bstone::file_flags_read | bstone::file_flags_exclusive, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file_3.is_open());
// } POSIX-lock fails here.
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Empty path.
void test_vy51tin1cn2agpui()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(!file.open("", bstone::file_flags_read, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Missing file.
void test_wrqnopermaxaz35r()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(!file.open(" ", bstone::file_flags_read, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Directory.
void test_ntnkh9imk7bb8qyu()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(!file.open(existing_dir, bstone::file_flags_read, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// No flags.
void test_h9t292cj96u2f9h3()
{
	create_test_data_file();
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(!file.open(test_data_file_name, bstone::file_flags_none, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// bool open(const char* path, FileFlags flags, FileErrorCode& error_code)
// Surrogate code point.
void test_8qlx2rm9h8q8hdwb()
{
	bstone::FileErrorCode error_code = bstone::ec_file_none;
	bstone::File file;
	tester.check(!file.open("\xED\xA0\x80", bstone::file_flags_read, error_code));
	tester.check(error_code != bstone::ec_file_none);
	tester.check(!file.is_open());
}

// ==========================================================================
// void close()
void test_ajqqx4t3372dtle8()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	file.close();
	tester.check(!file.is_open());
}

// ==========================================================================
// intptr_t read(void* buffer, intptr_t size) const
void test_rm8wnwh8150r2zz4()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	char buffer[10];
	tester.check(file.read(buffer, 5) == 5);
	tester.check(buffer[0] == '1');
	tester.check(buffer[1] == '2');
	tester.check(buffer[2] == '3');
	tester.check(buffer[3] == '4');
	tester.check(buffer[4] == '5');
	tester.check(file.read(buffer, 10) == 5);
	tester.check(buffer[0] == '6');
	tester.check(buffer[1] == '7');
	tester.check(buffer[2] == '8');
	tester.check(buffer[3] == '9');
	tester.check(buffer[4] == '0');
}

// ==========================================================================
// bool read_exactly(void* buffer, intptr_t size) const
void test_faawe13z981ao59p()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	char buffer[10];
	tester.check(file.read_exactly(buffer, 5));
	tester.check(buffer[0] == '1');
	tester.check(buffer[1] == '2');
	tester.check(buffer[2] == '3');
	tester.check(buffer[3] == '4');
	tester.check(buffer[4] == '5');
	tester.check(!file.read_exactly(buffer, 10));
	tester.check(file.read_exactly(buffer, 0));
}

// ==========================================================================
// intptr_t write(const void* buffer, intptr_t size) const
void test_qexfta3fmf2bfedw()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file.is_open());
	char buffer[5] = {'0', '9', '8', '7', '6'};
	tester.check(file.write(buffer, 5) == 5);
	tester.check(file.skip(-5) == 0);
	buffer[0] = '\0';
	buffer[1] = '\0';
	buffer[2] = '\0';
	buffer[3] = '\0';
	buffer[4] = '\0';
	tester.check(file.read(buffer, 5) == 5);
	tester.check(buffer[0] == '0');
	tester.check(buffer[1] == '9');
	tester.check(buffer[2] == '8');
	tester.check(buffer[3] == '7');
	tester.check(buffer[4] == '6');
}

// ==========================================================================
// bool write_exactly(const void* buffer, intptr_t size) const
void test_9i5ojyq4k78oz3w1()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file.is_open());
	char buffer[5] = {'0', '9', '8', '7', '6'};
	tester.check(file.write_exactly(buffer, 5));
	tester.check(file.skip(-5) == 0);
	buffer[0] = '\0';
	buffer[1] = '\0';
	buffer[2] = '\0';
	buffer[3] = '\0';
	buffer[4] = '\0';
	tester.check(file.read(buffer, 5) == 5);
	tester.check(buffer[0] == '0');
	tester.check(buffer[1] == '9');
	tester.check(buffer[2] == '8');
	tester.check(buffer[3] == '7');
	tester.check(buffer[4] == '6');
}

// ==========================================================================
// int64_t seek(int64_t offset, FileOrigin origin) const
void test_glimu4izzzpdq0ay()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.seek(100, bstone::file_origin_begin) == 100);
	tester.check(file.seek(0, bstone::file_origin_current) == 100);
	tester.check(file.seek(0, bstone::file_origin_end) == 10);
	tester.check(file.seek(-100, bstone::file_origin_begin) < 0);
	tester.check(file.seek(0, bstone::FileOrigin(-100)) < 0);
}

// ==========================================================================
// int64_t skip(int64_t offset) const
void test_wxw1wye9harta34h()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.skip(100) == 100);
	tester.check(file.skip(10) == 110);
	tester.check(file.skip(-50) == 60);
	tester.check(file.skip(-100) < 0);
}

// ==========================================================================
// int64_t get_position() const
void test_qcicap0ggsvn3q2a()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.set_position(100));
	tester.check(file.get_position() == 100);
	tester.check(file.seek(0, bstone::file_origin_end));
	tester.check(file.get_position() == 10);
}

// ==========================================================================
// bool set_position(int64_t position) const
void test_n1eqxrhnyzoddv2v()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.set_position(100));
	tester.check(file.get_position() == 100);
	tester.check(!file.set_position(-100));
	tester.check(file.get_position() == 100);
}

// ==========================================================================
// int64_t get_size() const
void test_aq4ovvt9bio5p5d4()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_read);
	tester.check(file.is_open());
	tester.check(file.get_size() == 10);
}

// ==========================================================================
// bool set_size(int64_t size) const
void test_tj9admzknt8uzfe7()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_write);
	tester.check(file.is_open());
	tester.check(file.get_size() == 10);
	tester.check(file.set_size(100));
	tester.check(file.get_size() == 100);
}

// ==========================================================================
// bool flush() const
void test_d9s4s2a0pxuqmvfj()
{
	create_test_data_file();
	bstone::File file(test_data_file_name, bstone::file_flags_write);
	tester.check(file.is_open());
	tester.check(file.flush());
}

// ==========================================================================
// bool lock(FileLockType lock_type) const
// Shared lock.
void test_y98do78y4khgs3ws()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_read_write));
	tester.check(file_1.is_open());
	tester.check(file_1.lock(bstone::file_lock_shared));
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2;
	tester.check(file_2.open(test_data_file_name, bstone::file_flags_read_write));
	tester.check(file_2.is_open());
	tester.check(file_2.lock(bstone::file_lock_shared));
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// bool lock(FileLockType lock_type) const
// Exclusive lock.
void test_un4mwys1we9b74im()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_1.is_open());
	tester.check(file_1.lock(bstone::file_lock_exclusive));
	tester.check(file_1.read_exactly(&buffer, 1));
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	bstone::File file_2(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_2.is_open());
	tester.check(!file_2.lock(bstone::file_lock_shared));

	bstone::File file_3(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_3.is_open());
	tester.check(!file_3.lock(bstone::file_lock_exclusive));
// } POSIX-lock fails here.
}

// ==========================================================================
// bool lock_shared() const
// Shared lock.
void test_pefnjcd15eft7u2y()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1;
	tester.check(file_1.open(test_data_file_name, bstone::file_flags_read_write));
	tester.check(file_1.is_open());
	tester.check(file_1.lock(bstone::file_lock_shared));
	tester.check(file_1.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_1.write_exactly(&buffer, 1));
#endif // _WIN32

	buffer = 0;
	bstone::File file_2;
	tester.check(file_2.open(test_data_file_name, bstone::file_flags_read_write));
	tester.check(file_2.is_open());
	tester.check(file_2.lock(bstone::file_lock_shared));
	tester.check(file_2.read_exactly(&buffer, 1));
	tester.check(buffer == '1');
#ifdef _WIN32 // POSIX-, BSD- and OFD-lock fails here.
	tester.check(!file_2.write_exactly(&buffer, 1));
#endif // _WIN32
}

// ==========================================================================
// bool lock_exclusive() const
// Exclusive lock.
void test_ube06tb0ufyf433d()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_1.is_open());
	tester.check(file_1.lock(bstone::file_lock_exclusive));
	tester.check(file_1.read_exactly(&buffer, 1));
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	bstone::File file_2(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_2.is_open());
	tester.check(!file_2.lock(bstone::file_lock_shared));

	bstone::File file_3(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_3.is_open());
	tester.check(!file_3.lock(bstone::file_lock_exclusive));
// } POSIX-lock fails here.
}

// ==========================================================================
// bool unlock() const
void test_kneumge3ryj9y94h()
{
	create_test_data_file();
	unsigned char buffer;

	buffer = 0;
	bstone::File file_1(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_1.is_open());
	tester.check(file_1.lock_exclusive());
	tester.check(file_1.read_exactly(&buffer, 1));
	buffer = '2';
	tester.check(file_1.write_exactly(&buffer, 1));

// { POSIX-lock fails here.
	bstone::File file_2(test_data_file_name, bstone::file_flags_read_write);
	tester.check(file_2.is_open());
	tester.check(!file_2.lock_shared());
	tester.check(!file_2.lock_exclusive());
// } POSIX-lock fails here.

	tester.check(file_1.unlock());

	tester.check(file_2.lock_exclusive());
}

// ==========================================================================
// void swap(File& file)
void test_ct54wh373ttvevhq()
{
	create_test_data_file();
	bstone::File file_1(test_data_file_name, bstone::file_flags_read);
	bstone::File file_2;
	tester.check(file_1.is_open());
	tester.check(!file_2.is_open());
	file_1.swap(file_2);
	tester.check(!file_1.is_open());
	tester.check(file_2.is_open());
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_ctor();
		register_is_open();
		register_open();
		register_close();
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
		register_lock();
		register_lock_shared();
		register_lock_exclusive();
		register_unlock();
		register_swap();
	}

private:
	void register_ctor()
	{
		//
		tester.register_test("File#ll78mzqsgafv1l26", test_ll78mzqsgafv1l26);

		//
		tester.register_test("File#lq63at77az23n5si", test_lq63at77az23n5si);
		tester.register_test("File#9kgqtp0jxn88o6sk", test_9kgqtp0jxn88o6sk);
		tester.register_test("File#ss1k6dv22myc6wd8", test_ss1k6dv22myc6wd8);
		tester.register_test("File#3w69uf8jer32pck6", test_3w69uf8jer32pck6);

		tester.register_test("File#aagzqydiws92d9p5", test_aagzqydiws92d9p5);
		tester.register_test("File#vxvnsqjp6plptr84", test_vxvnsqjp6plptr84);
		tester.register_test("File#w24wl6avxke60bwi", test_w24wl6avxke60bwi);
		tester.register_test("File#5v1hdbekrnfub8ax", test_5v1hdbekrnfub8ax);
		tester.register_test("File#fq3iip5r1ike73dz", test_fq3iip5r1ike73dz);
		tester.register_test("File#w7b4aj4c9c7s6h7d", test_w7b4aj4c9c7s6h7d);
		tester.register_test("File#s5lnjx4gwgom7gmx", test_s5lnjx4gwgom7gmx);

		tester.register_test("File#95kkrfvi96b3zx8h", test_95kkrfvi96b3zx8h);
		tester.register_test("File#x5qtvddnlo5tq9g1", test_x5qtvddnlo5tq9g1);
		tester.register_test("File#bkfaq0pwod1b8zl2", test_bkfaq0pwod1b8zl2);
		tester.register_test("File#0ab34jqqd4c340ll", test_0ab34jqqd4c340ll);
		tester.register_test("File#hpu1qzkaslmnmmof", test_hpu1qzkaslmnmmof);

		//
		tester.register_test("File#us9pymgvin6ju1dg", test_us9pymgvin6ju1dg);
		tester.register_test("File#1itv9h1lsc0q8tt9", test_1itv9h1lsc0q8tt9);
		tester.register_test("File#l3mosoqk31ffcjmp", test_l3mosoqk31ffcjmp);
		tester.register_test("File#hs0nf4joyxfjke1t", test_hs0nf4joyxfjke1t);

		tester.register_test("File#9k3guxydb4m5wxx9", test_9k3guxydb4m5wxx9);
		tester.register_test("File#awwos5is0tdxp96w", test_awwos5is0tdxp96w);
		tester.register_test("File#d3ogzymfmixkf15w", test_d3ogzymfmixkf15w);
		tester.register_test("File#thncsyg6wx4vte7x", test_thncsyg6wx4vte7x);
		tester.register_test("File#pf40iv19vmld85xk", test_pf40iv19vmld85xk);
		tester.register_test("File#djm5h61o0zt02c0k", test_djm5h61o0zt02c0k);
		tester.register_test("File#gvixklv93pr1il3k", test_gvixklv93pr1il3k);

		tester.register_test("File#a3bey5765ba9n5bc", test_a3bey5765ba9n5bc);
		tester.register_test("File#z7633fj9k8qtu7q7", test_z7633fj9k8qtu7q7);
		tester.register_test("File#zkh3gf4znr40prib", test_zkh3gf4znr40prib);
		tester.register_test("File#nio2oxxl2ycncdqr", test_nio2oxxl2ycncdqr);
		tester.register_test("File#1ml8olb35le5z411", test_1ml8olb35le5z411);
	}

	void register_is_open()
	{
		tester.register_test("File#vm2wofyyq3lqmkif", test_vm2wofyyq3lqmkif);
	}

	void register_open()
	{
		//
		tester.register_test("File#56fvardg84bl5zgr", test_56fvardg84bl5zgr);
		tester.register_test("File#ybzz4gqodkco7a6e", test_ybzz4gqodkco7a6e);
		tester.register_test("File#zebwonr8ctsvrqs3", test_zebwonr8ctsvrqs3);
		tester.register_test("File#gw6bveqqkje0t3co", test_gw6bveqqkje0t3co);

		tester.register_test("File#mwa9u1fbciegp6qw", test_mwa9u1fbciegp6qw);
		tester.register_test("File#s1kv571dol804hf3", test_s1kv571dol804hf3);
		tester.register_test("File#9eu656jx4efid17r", test_9eu656jx4efid17r);
		tester.register_test("File#2asori0d5innnejt", test_2asori0d5innnejt);
		tester.register_test("File#1gm81cu6jrgiz39y", test_1gm81cu6jrgiz39y);
		tester.register_test("File#q1hgucxoq4ezusqi", test_q1hgucxoq4ezusqi);
		tester.register_test("File#6stnrof91e93da36", test_6stnrof91e93da36);

		tester.register_test("File#oaernzfasw8uxg3q", test_oaernzfasw8uxg3q);
		tester.register_test("File#g299zwklmkjkk0xs", test_g299zwklmkjkk0xs);
		tester.register_test("File#o8097vrfgqon6uxt", test_o8097vrfgqon6uxt);
		tester.register_test("File#q3uux7jcmjtbfd6h", test_q3uux7jcmjtbfd6h);
		tester.register_test("File#vz681wc4jnviony9", test_vz681wc4jnviony9);

		//
		tester.register_test("File#sersynmko0no0w1k", test_sersynmko0no0w1k);
		tester.register_test("File#dq3pew0cd511rp3z", test_dq3pew0cd511rp3z);
		tester.register_test("File#g7rs30gvw6ilzopv", test_g7rs30gvw6ilzopv);
		tester.register_test("File#8sws6nqpqn9yr92m", test_8sws6nqpqn9yr92m);

		tester.register_test("File#6zqgkqjbol5grihn", test_6zqgkqjbol5grihn);
		tester.register_test("File#h01rlfpjkgsxwnst", test_h01rlfpjkgsxwnst);
		tester.register_test("File#3nrw1jq80pxkl6pf", test_3nrw1jq80pxkl6pf);
		tester.register_test("File#z6xf1bjtrvt5srfd", test_z6xf1bjtrvt5srfd);
		tester.register_test("File#gzf3t7kux7352cwj", test_gzf3t7kux7352cwj);
		tester.register_test("File#8k4vayfahmm5qlho", test_8k4vayfahmm5qlho);
		tester.register_test("File#p9r5hhb20dtig832", test_p9r5hhb20dtig832);

		tester.register_test("File#vy51tin1cn2agpui", test_vy51tin1cn2agpui);
		tester.register_test("File#wrqnopermaxaz35r", test_wrqnopermaxaz35r);
		tester.register_test("File#ntnkh9imk7bb8qyu", test_ntnkh9imk7bb8qyu);
		tester.register_test("File#h9t292cj96u2f9h3", test_h9t292cj96u2f9h3);
		tester.register_test("File#8qlx2rm9h8q8hdwb", test_8qlx2rm9h8q8hdwb);
	}

	void register_close()
	{
		tester.register_test("File#ajqqx4t3372dtle8", test_ajqqx4t3372dtle8);
	}

	void register_read()
	{
		tester.register_test("File#rm8wnwh8150r2zz4", test_rm8wnwh8150r2zz4);
	}

	void register_read_exactly()
	{
		tester.register_test("File#faawe13z981ao59p", test_faawe13z981ao59p);
	}

	void register_write()
	{
		tester.register_test("File#qexfta3fmf2bfedw", test_qexfta3fmf2bfedw);
	}

	void register_write_exactly()
	{
		tester.register_test("File#9i5ojyq4k78oz3w1", test_9i5ojyq4k78oz3w1);
	}

	void register_seek()
	{
		tester.register_test("File#glimu4izzzpdq0ay", test_glimu4izzzpdq0ay);
	}

	void register_skip()
	{
		tester.register_test("File#wxw1wye9harta34h", test_wxw1wye9harta34h);
	}

	void register_get_position()
	{
		tester.register_test("File#qcicap0ggsvn3q2a", test_qcicap0ggsvn3q2a);
	}

	void register_set_position()
	{
		tester.register_test("File#n1eqxrhnyzoddv2v", test_n1eqxrhnyzoddv2v);
	}

	void register_get_size()
	{
		tester.register_test("File#aq4ovvt9bio5p5d4", test_aq4ovvt9bio5p5d4);
	}

	void register_set_size()
	{
		tester.register_test("File#tj9admzknt8uzfe7", test_tj9admzknt8uzfe7);
	}

	void register_flush()
	{
		tester.register_test("File#d9s4s2a0pxuqmvfj", test_d9s4s2a0pxuqmvfj);
	}

	void register_lock()
	{
		tester.register_test("File#y98do78y4khgs3ws", test_y98do78y4khgs3ws);
		tester.register_test("File#un4mwys1we9b74im", test_un4mwys1we9b74im);
	}

	void register_lock_shared()
	{
		tester.register_test("File#pefnjcd15eft7u2y", test_pefnjcd15eft7u2y);
	}

	void register_lock_exclusive()
	{
		tester.register_test("File#ube06tb0ufyf433d", test_ube06tb0ufyf433d);
	}

	void register_unlock()
	{
		tester.register_test("File#kneumge3ryj9y94h", test_kneumge3ryj9y94h);
	}

	void register_swap()
	{
		tester.register_test("File#ct54wh373ttvevhq", test_ct54wh373ttvevhq);
	}
};

auto registrator = Registrator{};

} // namespace
