#include <cstddef>

#include <new>
#include <string>
#include <vector>

#include "zlib.h"

#include "bstone_archive_file.h"
#include "bstone_tester.h"
#include "bstone_zip_archive_file.h"
#include "bstone_zip_archive_file_crc_32.h"

namespace {

auto tester = bstone::Tester{};

// ==========================================================================

using Bytes = std::vector<unsigned char>;

constexpr auto local_file_header_signature = 0x04034B50U;
constexpr auto central_file_header_signature = 0x02014B50U;
constexpr auto end_of_central_dir_record_signature = 0x06054B50U;

constexpr auto compression_method_store = 0U;
constexpr auto compression_method_deflate = 8U;

void add_u16(Bytes& bytes, unsigned int value)
{
	bytes.push_back(static_cast<unsigned char>(value & 0xFFU));
	bytes.push_back(static_cast<unsigned char>((value >> 8) & 0xFFU));
}

void add_u32(Bytes& bytes, unsigned int value)
{
	add_u16(bytes, value & 0xFFFFU);
	add_u16(bytes, (value >> 16) & 0xFFFFU);
}

void add_bytes(Bytes& bytes, const Bytes& value)
{
	bytes.insert(bytes.end(), value.cbegin(), value.cend());
}

void add_name(Bytes& bytes, const std::string& name)
{
	for (const char ch : name)
		bytes.push_back(static_cast<unsigned char>(ch));
}

// An entry as it will be written out. The sizes are separate from the data on purpose:
// a malformed archive is one where they disagree.
struct TestZipEntry
{
	std::string name{};
	unsigned int compression_method{};
	unsigned int crc_32{};
	unsigned int compressed_size{};
	unsigned int uncompressed_size{};
	Bytes data{};
	Bytes extra_field{}; // Central header only.
};

// Assembles a complete archive. A non-negative "dir_total_entries" overrides the count the
// end of central directory record promises, which is otherwise the real one.
Bytes make_test_zip(const std::vector<TestZipEntry>& entries, int dir_total_entries = -1)
{
	auto bytes = Bytes{};
	auto local_header_offsets = std::vector<unsigned int>{};
	for (const TestZipEntry& entry : entries)
	{
		local_header_offsets.push_back(static_cast<unsigned int>(bytes.size()));
		add_u32(bytes, local_file_header_signature);
		add_u16(bytes, 20); // version needed
		add_u16(bytes, 0); // flags
		add_u16(bytes, entry.compression_method);
		add_u16(bytes, 0); // time
		add_u16(bytes, 0); // date
		add_u32(bytes, entry.crc_32);
		add_u32(bytes, entry.compressed_size);
		add_u32(bytes, entry.uncompressed_size);
		add_u16(bytes, static_cast<unsigned int>(entry.name.size()));
		add_u16(bytes, 0); // extra field length
		add_name(bytes, entry.name);
		add_bytes(bytes, entry.data);
	}
	const auto dir_offset = static_cast<unsigned int>(bytes.size());
	for (std::size_t i = 0; i < entries.size(); ++i)
	{
		const TestZipEntry& entry = entries[i];
		add_u32(bytes, central_file_header_signature);
		add_u16(bytes, 20); // version made by
		add_u16(bytes, 20); // version needed
		add_u16(bytes, 0); // flags
		add_u16(bytes, entry.compression_method);
		add_u16(bytes, 0); // time
		add_u16(bytes, 0); // date
		add_u32(bytes, entry.crc_32);
		add_u32(bytes, entry.compressed_size);
		add_u32(bytes, entry.uncompressed_size);
		add_u16(bytes, static_cast<unsigned int>(entry.name.size()));
		add_u16(bytes, static_cast<unsigned int>(entry.extra_field.size()));
		add_u16(bytes, 0); // file comment length
		add_u16(bytes, 0); // disk number start
		add_u16(bytes, 0); // internal attributes
		add_u32(bytes, 0); // external attributes
		add_u32(bytes, local_header_offsets[i]);
		add_name(bytes, entry.name);
		add_bytes(bytes, entry.extra_field);
	}
	const auto dir_size = static_cast<unsigned int>(bytes.size()) - dir_offset;
	const auto total_entries = dir_total_entries >= 0
		? static_cast<unsigned int>(dir_total_entries)
		: static_cast<unsigned int>(entries.size());
	add_u32(bytes, end_of_central_dir_record_signature);
	add_u16(bytes, 0); // this disk number
	add_u16(bytes, 0); // dir disk number
	add_u16(bytes, total_entries);
	add_u16(bytes, total_entries);
	add_u32(bytes, dir_size);
	add_u32(bytes, dir_offset);
	add_u16(bytes, 0); // comment length
	return bytes;
}

bool open_test_zip(bstone::ArchiveFile& archive_file, const Bytes& bytes)
{
	return archive_file.open_memory(bytes.data(), static_cast<int>(bytes.size()));
}

unsigned int zip_crc_32(const Bytes& bytes)
{
	auto crc_32 = bstone::ZipArchiveFileCrc32{};
	crc_32.reset();
	crc_32.update(bytes.data(), static_cast<int>(bytes.size()));
	crc_32.finish();
	return crc_32.get_value();
}

Bytes make_pattern_bytes(int size)
{
	auto bytes = Bytes(static_cast<std::size_t>(size));
	for (int i = 0; i < size; ++i)
		bytes[static_cast<std::size_t>(i)] = static_cast<unsigned char>(i & 0x0F);
	return bytes;
}

// The bundled zlib is built with Z_SOLO, so it has no allocator of its own.
::voidpf zlib_alloc_func(::voidpf, ::uInt items, ::uInt size)
{
	return ::operator new(std::size_t{items} * size, std::nothrow);
}

void zlib_free_func(::voidpf, ::voidpf address)
{
	::operator delete(address);
}

Bytes deflate_bytes(const Bytes& src)
{
	auto stream = ::z_stream{};
	stream.zalloc = zlib_alloc_func;
	stream.zfree = zlib_free_func;
	if (::deflateInit2(&stream, Z_BEST_COMPRESSION, Z_DEFLATED, -15, 8, Z_DEFAULT_STRATEGY) != Z_OK)
		return Bytes{};
	auto dst = Bytes(src.size() + 1024);
	stream.next_in = src.data();
	stream.avail_in = static_cast<::uInt>(src.size());
	stream.next_out = dst.data();
	stream.avail_out = static_cast<::uInt>(dst.size());
	const int deflate_result = ::deflate(&stream, Z_FINISH);
	const auto dst_size = static_cast<std::size_t>(stream.total_out);
	static_cast<void>(::deflateEnd(&stream));
	if (deflate_result != Z_STREAM_END)
		return Bytes{};
	dst.resize(dst_size);
	return dst;
}

TestZipEntry make_stored_entry(const std::string& name, const Bytes& data)
{
	auto entry = TestZipEntry{};
	entry.name = name;
	entry.compression_method = compression_method_store;
	entry.crc_32 = zip_crc_32(data);
	entry.compressed_size = static_cast<unsigned int>(data.size());
	entry.uncompressed_size = static_cast<unsigned int>(data.size());
	entry.data = data;
	return entry;
}

TestZipEntry make_deflated_entry(const std::string& name, const Bytes& data)
{
	auto entry = TestZipEntry{};
	entry.name = name;
	entry.compression_method = compression_method_deflate;
	entry.crc_32 = zip_crc_32(data);
	entry.data = deflate_bytes(data);
	entry.compressed_size = static_cast<unsigned int>(entry.data.size());
	entry.uncompressed_size = static_cast<unsigned int>(data.size());
	return entry;
}

// ==========================================================================

// bool open_memory(const void*, int)
// A well-formed archive with one stored entry.
void test_q7m3xk1p9wz0aebt()
{
	const auto data = make_pattern_bytes(64);
	const auto zip = make_test_zip({make_stored_entry("assets/one.txt", data)});
	const auto archive_file = bstone::make_zip_archive_file();
	const auto is_open = open_test_zip(*archive_file, zip);
	tester.check(
		is_open &&
		archive_file->get_entry_count() == 1 &&
		std::string{archive_file->get_entry(0).name} == "assets/one.txt" &&
		archive_file->get_entry(0).name_length == 14 &&
		archive_file->get_entry(0).uncompressed_size == 64 &&
		!archive_file->get_entry(0).is_compressed);
}

// bool open_memory(const void*, int)
// The directory promises more entries than it has room for.
void test_h4nv82rjc6ylsq5d()
{
	const auto data = make_pattern_bytes(64);
	const auto zip = make_test_zip({make_stored_entry("assets/one.txt", data)}, 2);
	const auto archive_file = bstone::make_zip_archive_file();
	tester.check(!open_test_zip(*archive_file, zip));
}

// bool open_memory(const void*, int)
// The directory promises the largest entry count a non-ZIP64 record can hold.
void test_z0f9tk3mwq7bxr14()
{
	const auto data = make_pattern_bytes(64);
	const auto zip = make_test_zip({make_stored_entry("assets/one.txt", data)}, 0xFFFE);
	const auto archive_file = bstone::make_zip_archive_file();
	tester.check(!open_test_zip(*archive_file, zip));
}

// bool open_memory(const void*, int)
// The directory is big enough for the promised entries yet its names do not fit the arena
// those entries leave room for.
void test_p6ay5dn8hjv2ceu0()
{
	const auto data = make_pattern_bytes(16);
	const auto name = std::string(64, 'n');
	const auto zip = make_test_zip({make_stored_entry(name, data)}, 2);
	const auto archive_file = bstone::make_zip_archive_file();
	tester.check(!open_test_zip(*archive_file, zip));
}

// bool open_memory(const void*, int)
// An extra field holding one well-formed record.
void test_r3wq7lz1o9skmv6t()
{
	const auto data = make_pattern_bytes(16);
	auto entry = make_stored_entry("one.txt", data);
	entry.extra_field = Bytes{0x0A, 0x00, 0x04, 0x00, 0x01, 0x02, 0x03, 0x04};
	const auto zip = make_test_zip({entry});
	const auto archive_file = bstone::make_zip_archive_file();
	const auto is_open = open_test_zip(*archive_file, zip);
	tester.check(is_open && archive_file->get_entry_count() == 1);
}

// bool open_memory(const void*, int)
// An extra record whose declared size runs past the extra field it lives in. It stops just
// inside the directory buffer, on the last byte of the header that follows.
void test_b8xu4gyc2n5jhwq3()
{
	const auto data = make_pattern_bytes(16);
	auto first_entry = make_stored_entry("a.txt", data);
	first_entry.extra_field = Bytes{0x0A, 0x00, 0x37, 0x00, 0x01, 0x02, 0x03, 0x04};
	const auto second_entry = make_stored_entry("b.txt", data);
	const auto zip = make_test_zip({first_entry, second_entry}, 1);
	const auto archive_file = bstone::make_zip_archive_file();
	tester.check(!open_test_zip(*archive_file, zip));
}

// bool open_memory(const void*, int)
// A stored entry whose two sizes disagree.
void test_k9td1rvp7f0mzsae()
{
	const auto data = make_pattern_bytes(64);
	auto entry = make_stored_entry("one.txt", data);
	entry.compressed_size = 0;
	const auto zip = make_test_zip({entry});
	const auto archive_file = bstone::make_zip_archive_file();
	const auto is_open = open_test_zip(*archive_file, zip);
	tester.check(is_open && archive_file->get_entry_count() == 0);
}

// ==========================================================================

// ArchiveFileEntryStreamUPtr open_entry_stream(int) const
// A stored entry reads back byte for byte.
void test_c5ho3jw8qylb2unx()
{
	const auto data = make_pattern_bytes(64);
	const auto zip = make_test_zip({make_stored_entry("one.txt", data)});
	const auto archive_file = bstone::make_zip_archive_file();
	if (!open_test_zip(*archive_file, zip))
		tester.fail("Failed to open the archive.");
	const auto stream = archive_file->open_entry_stream(0);
	if (stream == nullptr)
		tester.fail("Failed to open the entry stream.");
	auto actual_data = Bytes(data.size());
	const auto read_size = stream->read(actual_data.data(), static_cast<int>(actual_data.size()));
	tester.check(stream->get_size() == 64 && read_size == 64 && actual_data == data);
}

// ArchiveFileEntryStreamUPtr open_entry_stream(int) const
// A stored entry rewound after a partial read hashes only the second pass.
void test_w2ke7sza4vd9tlq6()
{
	const auto data = make_pattern_bytes(64);
	const auto zip = make_test_zip({make_stored_entry("one.txt", data)});
	const auto archive_file = bstone::make_zip_archive_file();
	if (!open_test_zip(*archive_file, zip))
		tester.fail("Failed to open the archive.");
	const auto stream = archive_file->open_entry_stream(0);
	if (stream == nullptr)
		tester.fail("Failed to open the entry stream.");
	auto partial_data = Bytes(16);
	const auto partial_read_size = stream->read(partial_data.data(), static_cast<int>(partial_data.size()));
	const auto is_rewound = stream->rewind();
	auto actual_data = Bytes(data.size());
	const auto read_size = stream->read(actual_data.data(), static_cast<int>(actual_data.size()));
	tester.check(partial_read_size == 16 && is_rewound && read_size == 64 && actual_data == data);
}

// ArchiveFileEntryStreamUPtr open_entry_stream(int) const
// A deflated entry reads back byte for byte.
void test_n1cb6mfxr8u3ypgo()
{
	const auto data = make_pattern_bytes(4096);
	const auto zip = make_test_zip({make_deflated_entry("one.txt", data)});
	const auto archive_file = bstone::make_zip_archive_file();
	if (!open_test_zip(*archive_file, zip))
		tester.fail("Failed to open the archive.");
	const auto stream = archive_file->open_entry_stream(0);
	if (stream == nullptr)
		tester.fail("Failed to open the entry stream.");
	auto actual_data = Bytes(data.size());
	const auto read_size = stream->read(actual_data.data(), static_cast<int>(actual_data.size()));
	tester.check(stream->get_size() == 4096 && read_size == 4096 && actual_data == data);
}

// ArchiveFileEntryStreamUPtr open_entry_stream(int) const
// A deflated entry rewound while compressed bytes are still buffered decodes from the
// beginning again.
void test_y4jq0ei5tsl7dvzc()
{
	const auto data = make_pattern_bytes(32768);
	const auto zip = make_test_zip({make_deflated_entry("one.txt", data)});
	const auto archive_file = bstone::make_zip_archive_file();
	if (!open_test_zip(*archive_file, zip))
		tester.fail("Failed to open the archive.");
	const auto stream = archive_file->open_entry_stream(0);
	if (stream == nullptr)
		tester.fail("Failed to open the entry stream.");
	auto partial_data = Bytes(16);
	const auto partial_read_size = stream->read(partial_data.data(), static_cast<int>(partial_data.size()));
	const auto is_rewound = stream->rewind();
	auto actual_data = Bytes(data.size());
	const auto read_size = stream->read(actual_data.data(), static_cast<int>(actual_data.size()));
	tester.check(partial_read_size == 16 && is_rewound && read_size == 32768 && actual_data == data);
}

// ArchiveFileEntryStreamUPtr open_entry_stream(int) const
// A deflated entry that decodes to less than it promises stops instead of spinning.
void test_g7pz9khn3bwr1amx()
{
	const auto data = make_pattern_bytes(4);
	auto entry = make_deflated_entry("one.txt", data);
	entry.uncompressed_size = 100000;
	const auto zip = make_test_zip({entry});
	const auto archive_file = bstone::make_zip_archive_file();
	if (!open_test_zip(*archive_file, zip))
		tester.fail("Failed to open the archive.");
	const auto stream = archive_file->open_entry_stream(0);
	if (stream == nullptr)
		tester.fail("Failed to open the entry stream.");
	auto actual_data = Bytes(100000);
	const auto read_size = stream->read(actual_data.data(), static_cast<int>(actual_data.size()));
	auto extra_byte = Bytes(1);
	const auto extra_read_size = stream->read(extra_byte.data(), 1);
	actual_data.resize(read_size > 0 ? static_cast<std::size_t>(read_size) : 0);
	tester.check(read_size == 4 && actual_data == data && extra_read_size == 0);
}

// ==========================================================================

class Registrator
{
public:
	Registrator()
	{
		register_open_memory();
		register_open_entry_stream();
	}

private:
	void register_open_memory()
	{
		tester.register_test("ZipArchiveFile#q7m3xk1p9wz0aebt", test_q7m3xk1p9wz0aebt);
		tester.register_test("ZipArchiveFile#h4nv82rjc6ylsq5d", test_h4nv82rjc6ylsq5d);
		tester.register_test("ZipArchiveFile#z0f9tk3mwq7bxr14", test_z0f9tk3mwq7bxr14);
		tester.register_test("ZipArchiveFile#p6ay5dn8hjv2ceu0", test_p6ay5dn8hjv2ceu0);
		tester.register_test("ZipArchiveFile#r3wq7lz1o9skmv6t", test_r3wq7lz1o9skmv6t);
		tester.register_test("ZipArchiveFile#b8xu4gyc2n5jhwq3", test_b8xu4gyc2n5jhwq3);
		tester.register_test("ZipArchiveFile#k9td1rvp7f0mzsae", test_k9td1rvp7f0mzsae);
	}

	void register_open_entry_stream()
	{
		tester.register_test("ZipArchiveFile#c5ho3jw8qylb2unx", test_c5ho3jw8qylb2unx);
		tester.register_test("ZipArchiveFile#w2ke7sza4vd9tlq6", test_w2ke7sza4vd9tlq6);
		tester.register_test("ZipArchiveFile#n1cb6mfxr8u3ypgo", test_n1cb6mfxr8u3ypgo);
		tester.register_test("ZipArchiveFile#y4jq0ei5tsl7dvzc", test_y4jq0ei5tsl7dvzc);
		tester.register_test("ZipArchiveFile#g7pz9khn3bwr1amx", test_g7pz9khn3bwr1amx);
	}
};

auto registrator = Registrator{};

} // namespace
