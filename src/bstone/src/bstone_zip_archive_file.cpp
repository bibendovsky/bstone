/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_zip_archive_file.h"
#include <algorithm>
#include "bstone_archive_file_input_stream.h"
#include "bstone_assert.h"
#include "bstone_endian.h"
#include "bstone_memory_binary_reader.h"

namespace bstone {

ArchiveFileEntryStreamUPtr make_archive_file_inflate_entry_stream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry);

ArchiveFileEntryStreamUPtr make_archive_file_store_entry_stream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry);

// =====================================

namespace {

class ZipArchiveFile final : public ArchiveFile
{
public:
	ZipArchiveFile() = default;
	~ZipArchiveFile() override = default;

	bool open_file(const char* file_path) override;
	bool open_memory(const void* buffer, int size) override;
	void close() override;
	bool is_open() const override;
	int get_entry_count() const override;
	const ArchiveFileEntry& get_entry(int entry_index) const override;
	ArchiveFileEntryStreamUPtr open_entry_stream(int entry_index) const override;

private:
	inline static constexpr int max_central_dir_size = 16 * 1024 * 1024;

	inline static constexpr int local_file_header_size = 30;
	inline static constexpr int central_file_header_size = 46;
	inline static constexpr int end_of_central_dir_record_size = 22;

	inline static constexpr int extra_record_id_language_encoding = 0x0008; // Language Encoding (EFS)
	inline static constexpr int extra_record_id_info_zip_unicode_path = 0x7075; // Info-ZIP Unicode Path Extra Field

	inline static constexpr unsigned int file_encrypted_flag = 1U << 0;
	inline static constexpr unsigned int data_descriptor_flag = 1U << 3;
	inline static constexpr unsigned int patched_data_flag = 1U << 5;
	inline static constexpr unsigned int strong_encryption_flag = 1U << 6;
	inline static constexpr unsigned int language_flag = 1U << 11;
	inline static constexpr unsigned int dir_encrypted_flag = 1U << 13;

	inline static constexpr int compression_method_store = 0;
	inline static constexpr int compression_method_deflate = 8;

	inline static constexpr int zip64_marker_16 = 0xFFFF;
	inline static constexpr unsigned int zip64_marker_32 = 0xFFFFFFFFU;

	inline static constexpr unsigned int max_data_size = INT32_MAX;

private:
	struct RawMemoryDeleter
	{
		void operator()(void* ptr) const
		{
			::operator delete(ptr);
		}
	};

	using Names = std::unique_ptr<char, RawMemoryDeleter>;
	using Entries = std::unique_ptr<ArchiveFileEntry[], RawMemoryDeleter>;

	struct OpenFileParam
	{
		const char* file_path;
	};

	struct OpenMemoryParam
	{
		const void* buffer;
		int size;
	};

	struct OpenParam
	{
		bool is_file;

		union
		{
			OpenFileParam file;
			OpenMemoryParam memory;
		};
	};

	struct LocalFileHeader
	{
		unsigned int signature;
		int version_needed;
		unsigned int flags;
		int compression_method;
		int last_mod_file_time;
		int last_mod_file_date;
		unsigned int crc_32;
		unsigned int compressed_size;
		unsigned int uncompressed_size;
		int file_name_length;
		int extra_field_length;
	};

	struct CentralFileHeader
	{
		unsigned int signature;
		int version_made_by;
		int version_needed;
		unsigned int flags;
		int compression_method;
		int last_mod_file_time;
		int last_mod_file_date;
		unsigned int crc_32;
		unsigned int compressed_size;
		unsigned int uncompressed_size;
		int file_name_length;
		int extra_field_length;
		int file_comment_length;
		int disk_number_start;
		unsigned int internal_file_attributes;
		unsigned int external_file_attributes;
		unsigned int local_header_offset;
		const char* file_name;
		bool has_extended_language_encoding_data;
		bool has_info_zip_unicode_path_extra_field;
	};

	struct EndOfCentralDirRecord
	{
		unsigned int signature;
		int this_disk_number;
		int dir_disk_number;
		int this_total_entries;
		int dir_total_entries;
		unsigned int dir_size;
		unsigned int dir_offset;
		int comment_length;
	};

	using BufferUPtr = std::unique_ptr<void, RawMemoryDeleter>;

	bool is_open_{};
	ArchiveFileInputStreamUPtr input_stream_{};
	Names names_{};
	int names_capacity_{};
	int names_size_{};
	Entries entries_{};
	int entries_capacity_{};
	int entries_size_{};

	template<typename T>
	static T align_16(T value)
	{
		return ((value + 15) / 16) * 16;
	}

	static void deserialize_local_file_header(MemoryBinaryReader& binary_reader, LocalFileHeader& header);
	static bool validate_local_file_header(const LocalFileHeader& header, const ArchiveFileEntry& entry);
	static void deserialize_end_of_central_dir_record(MemoryBinaryReader& binary_reader, EndOfCentralDirRecord& record);
	bool validate_end_of_central_dir_record(const EndOfCentralDirRecord& record) const;
	bool read_end_of_central_dir_record(EndOfCentralDirRecord& record);
	bool deserialize_central_file_header(MemoryBinaryReader& binary_reader, CentralFileHeader& header);
	bool validate_central_file_header(const CentralFileHeader& header) const;
	bool is_central_file_supported(const CentralFileHeader& header) const;
	bool initialize_entry_name(const CentralFileHeader& header, ArchiveFileEntry& entry);
	void reserve_entries(int capacity);
	bool add_entry(const CentralFileHeader& header);
	void reserve_names(int capacity);
	bool read_central_dir(const EndOfCentralDirRecord& eocdr);
	bool prepare_input_stream_for_entry(ArchiveFileInputStream& input_stream, const ArchiveFileEntry& entry) const;
	bool impl_open();
	void impl_close();
	bool impl_is_open() const;
	bool impl_open(const OpenParam& param);
	int impl_get_entry_count() const;
};

// --------------------------------------

bool ZipArchiveFile::open_file(const char* file_path)
{
	BSTONE_ASSERT(file_path != nullptr);
	return impl_open(
		OpenParam{
			.is_file = true,
			.file = OpenFileParam{.file_path = file_path}});
}

bool ZipArchiveFile::open_memory(const void* buffer, int size)
{
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(size >= 0);
	return impl_open(
		OpenParam{
			.is_file = false,
			.memory = OpenMemoryParam{
				.buffer = buffer,
				.size = size}});
}

void ZipArchiveFile::close()
{
	impl_close();
}

bool ZipArchiveFile::is_open() const
{
	return impl_is_open();
}

int ZipArchiveFile::get_entry_count() const
{
	BSTONE_ASSERT(impl_is_open());
	return impl_get_entry_count();
}

const ArchiveFileEntry& ZipArchiveFile::get_entry(int entry_index) const
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(entry_index >= 0 && entry_index < impl_get_entry_count());
	return entries_[entry_index];
}

ArchiveFileEntryStreamUPtr ZipArchiveFile::open_entry_stream(int entry_index) const
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(entry_index >= 0 && entry_index < impl_get_entry_count());
	const ArchiveFileEntry& entry = entries_[entry_index];
	ArchiveFileInputStreamUPtr entry_input_stream{input_stream_->open_from_this()};
	if (entry_input_stream == nullptr)
		return nullptr;
	if (!prepare_input_stream_for_entry(*entry_input_stream, entry))
		return nullptr;
	if (entry.compression_method == ArchiveFileCompressionMethod::store)
		return make_archive_file_store_entry_stream(std::move(entry_input_stream), entry);
	return make_archive_file_inflate_entry_stream(std::move(entry_input_stream), entry);
}

void ZipArchiveFile::deserialize_local_file_header(MemoryBinaryReader& binary_reader, LocalFileHeader& header)
{
	BSTONE_ASSERT(binary_reader.can_read_n(local_file_header_size));
	// local file header signature     4 bytes  (0x04034b50)
	header.signature          = binary_reader.read_u32_le();
	// version needed to extract       2 bytes
	header.version_needed     = binary_reader.read_u16_le();
	// general purpose bit flag        2 bytes
	header.flags              = binary_reader.read_u16_le();
	// compression method              2 bytes
	header.compression_method = binary_reader.read_u16_le();
	// last mod file time              2 bytes
	header.last_mod_file_time = binary_reader.read_u16_le();
	// last mod file date              2 bytes
	header.last_mod_file_date = binary_reader.read_u16_le();
	// crc-32                          4 bytes
	header.crc_32             = binary_reader.read_u32_le();
	// compressed size                 4 bytes
	header.compressed_size    = binary_reader.read_u32_le();
	// uncompressed size               4 bytes
	header.uncompressed_size  = binary_reader.read_u32_le();
	// file name length                2 bytes
	header.file_name_length   = binary_reader.read_u16_le();
	// extra field length              2 bytes
	header.extra_field_length = binary_reader.read_u16_le();
}

// Everything checked here was read out of the archive, which is a file the user was given
// and dropped into a search path. A header that fails to make sense is a rejected archive,
// not a programming error, so none of these report themselves by asserting: a debug build
// would abort on a file the release build merely refuses.
bool ZipArchiveFile::validate_local_file_header(const LocalFileHeader& header, const ArchiveFileEntry& entry)
{
	if (header.signature != 0x04034B50U)
		return false;
	if (((header.flags & file_encrypted_flag) != 0) ||
		((header.flags & patched_data_flag) != 0) ||
		((header.flags & strong_encryption_flag) != 0) ||
		((header.flags & dir_encrypted_flag) != 0))
	{
		return false;
	}
	const bool is_utf8 = (header.flags & language_flag) != 0;
	const ArchiveFileLanguageEncoding language_encoding =
		is_utf8 ? ArchiveFileLanguageEncoding::utf8 : ArchiveFileLanguageEncoding::cp437;
	if (language_encoding != entry.language_encoding)
		return false;
	ArchiveFileCompressionMethod compression_method;
	switch (header.compression_method)
	{
		case compression_method_store:
			compression_method = ArchiveFileCompressionMethod::store;
			break;
		case compression_method_deflate:
			compression_method = ArchiveFileCompressionMethod::deflate;
			break;
		default:
			compression_method = ArchiveFileCompressionMethod::unknown;
			break;
	}
	if (compression_method != entry.compression_method)
		return false;
	const bool has_data_descriptor = (header.flags & data_descriptor_flag) != 0;
	if (!has_data_descriptor)
	{
		if (header.crc_32 != entry.crc_32)
			return false;
		if (header.compressed_size != static_cast<unsigned int>(entry.compressed_size))
			return false;
		if (header.uncompressed_size != static_cast<unsigned int>(entry.uncompressed_size))
			return false;
	}
	if (header.file_name_length != entry.name_length)
		return false;
	return true;
}

void ZipArchiveFile::deserialize_end_of_central_dir_record(MemoryBinaryReader& binary_reader, EndOfCentralDirRecord& record)
{
	BSTONE_ASSERT(binary_reader.can_read_n(end_of_central_dir_record_size));
	// end of central dir signature    4 bytes  (0x06054b50)
	record.signature          = binary_reader.read_u32_le();
	// number of this disk             2 bytes
	record.this_disk_number   = binary_reader.read_u16_le();
	// number of the disk with the
	// start of the central directory  2 bytes
	record.dir_disk_number    = binary_reader.read_u16_le();
	// total number of entries in the
	// central directory on this disk  2 bytes
	record.this_total_entries = binary_reader.read_u16_le();
	// total number of entries in
	// the central directory           2 bytes
	record.dir_total_entries  = binary_reader.read_u16_le();
	// size of the central directory   4 bytes
	record.dir_size           = binary_reader.read_u32_le();
	// offset of start of central
	// directory with respect to
	// the starting disk number        4 bytes
	record.dir_offset         = binary_reader.read_u32_le();
	// .ZIP file comment length        2 bytes
	record.comment_length     = binary_reader.read_u16_le();
	// .ZIP file comment       (variable size)
	// (skip)
}

bool ZipArchiveFile::validate_end_of_central_dir_record(const EndOfCentralDirRecord& record) const
{
	// A ZIP64 archive, a multi-disk one and a self-contradicting one are all simply
	// unsupported input, so they are refused the same way an unreadable file would be.
	if (record.this_disk_number != 0)
		return false;
	if (record.dir_disk_number != 0)
		return false;
	if (record.this_total_entries == zip64_marker_16)
		return false;
	if (record.dir_total_entries == zip64_marker_16)
		return false;
	if (record.this_total_entries != record.dir_total_entries)
		return false;
	if (record.dir_size == zip64_marker_32)
		return false;
	if (record.dir_size > max_central_dir_size)
		return false;
	// The entry name arena is sized by subtracting the fixed part of every promised header
	// from the directory size. Nothing else ties the two fields together, so a directory
	// too small to hold the entries it promises would undersize the arena.
	if (record.dir_size < static_cast<unsigned int>(central_file_header_size * record.dir_total_entries))
		return false;
	if (record.dir_offset == zip64_marker_32)
		return false;
	return true;
}

bool ZipArchiveFile::read_end_of_central_dir_record(EndOfCentralDirRecord& record)
{
	const long long archive_size = input_stream_->get_size();
	if (archive_size < 0)
		return false;
	if (archive_size < end_of_central_dir_record_size)
		return false;
	constexpr int max_comment_length = 0xFFFF;
	constexpr int history_size = end_of_central_dir_record_size - 1;
	constexpr long long max_scan_size = end_of_central_dir_record_size + max_comment_length;
	const long long archive_end_position = archive_size - 1;
	const long long end_position = std::max(archive_end_position - max_scan_size, 0LL);
	constexpr int cache_capacity = 256;
	static_assert(cache_capacity > 0);
	unsigned char cache[cache_capacity + history_size]{}; // Data is followed by the history.
	for (long long position = archive_end_position; position >= end_position; )
	{
		const int cache_size = std::min(static_cast<int>(position - end_position), cache_capacity);
		if (cache_size == 0)
			return false;
		// Preserve the history.
		if (cache_size >= history_size)
			std::copy_n(cache, history_size, cache + cache_size);
		else
			std::copy_backward(cache, cache + history_size, cache + cache_size + history_size);
		const long long read_position = position - cache_size;
		if (!input_stream_->set_position(read_position) ||
			!input_stream_->read_exactly(cache, cache_size))
		{
			return false;
		}
		for (int cache_offset = cache_size - 1; cache_offset >= 0; --cache_offset)
		{
			const unsigned char* const raw_record = cache + cache_offset;
			const bool found_signature = (
				raw_record[0] == 0x50 &&
				raw_record[1] == 0x4B &&
				raw_record[2] == 0x05 &&
				raw_record[3] == 0x06);
			--position;
			if (found_signature)
			{
				MemoryBinaryReader binary_reader{raw_record, end_of_central_dir_record_size};
				deserialize_end_of_central_dir_record(binary_reader, record);
				return true;
			}
		}
	}
	return false;
}

bool ZipArchiveFile::deserialize_central_file_header(MemoryBinaryReader& binary_reader, CentralFileHeader& header)
{
	// Truncation anywhere below is a malformed archive, which the return value already
	// expresses; asserting would abort a debug build on a file the user merely has to be
	// told is unusable.
	if (!binary_reader.can_read_n(central_file_header_size))
		return false;
	// central file header signature   4 bytes  (0x02014B50)
	header.signature                = binary_reader.read_u32_le();
	// version made by                 2 bytes
	header.version_made_by          = binary_reader.read_u16_le();
	// version needed to extract       2 bytes
	header.version_needed           = binary_reader.read_u16_le();
	// general purpose bit flag        2 bytes
	header.flags                    = binary_reader.read_u16_le();
	// compression method              2 bytes
	header.compression_method       = binary_reader.read_u16_le();
	// last mod file time              2 bytes
	header.last_mod_file_time       = binary_reader.read_u16_le();
	// last mod file date              2 bytes
	header.last_mod_file_date       = binary_reader.read_u16_le();
	// crc-32                          4 bytes
	header.crc_32                   = binary_reader.read_u32_le();
	// compressed size                 4 bytes
	header.compressed_size          = binary_reader.read_u32_le();
	// uncompressed size               4 bytes
	header.uncompressed_size        = binary_reader.read_u32_le();
	// file name length                2 bytes
	header.file_name_length         = binary_reader.read_u16_le();
	// extra field length              2 bytes
	header.extra_field_length       = binary_reader.read_u16_le();
	// file comment length             2 bytes
	header.file_comment_length      = binary_reader.read_u16_le();
	// disk number start               2 bytes
	header.disk_number_start        = binary_reader.read_u16_le();
	// internal file attributes        2 bytes
	header.internal_file_attributes = binary_reader.read_u16_le();
	// external file attributes        4 bytes
	header.external_file_attributes = binary_reader.read_u32_le();
	// relative offset of local header 4 bytes
	header.local_header_offset      = binary_reader.read_u32_le();
	// file name (variable size)
	if (!binary_reader.can_read_n(header.file_name_length))
		return false;
	header.file_name = static_cast<const char*>(binary_reader.get_current_data());
	binary_reader.skip(header.file_name_length);
	// extra field (variable size)
	if (!binary_reader.can_read_n(header.extra_field_length))
		return false;
	// Look up for language-related extra fields.
	header.has_extended_language_encoding_data = false;
	header.has_info_zip_unicode_path_extra_field = false;
	for (int extra_field_offset = 0; extra_field_offset < header.extra_field_length; )
	{
		if (!binary_reader.can_read_n(4))
			return false;
		const int extra_header_id = binary_reader.read_u16_le();
		const int extra_header_size = binary_reader.read_u16_le();
		switch (extra_header_id)
		{
			case extra_record_id_language_encoding:
				header.has_extended_language_encoding_data = true;
				break;
			case extra_record_id_info_zip_unicode_path:
				header.has_info_zip_unicode_path_extra_field = true;
				break;
		}
		if (!binary_reader.can_read_n(extra_header_size))
			return false;
		binary_reader.skip(extra_header_size);
		extra_field_offset += 4 + extra_header_size;
	}
	// file comment (variable size)
	if (!binary_reader.can_read_n(header.file_comment_length))
		return false;
	binary_reader.skip(header.file_comment_length);
	//
	return true;
}

bool ZipArchiveFile::validate_central_file_header(const CentralFileHeader& header) const
{
	if (header.signature != 0x02014B50U)
		return false;
	if (header.disk_number_start != 0)
		return false;
	return true;
}

bool ZipArchiveFile::is_central_file_supported(const CentralFileHeader& header) const
{
	if (((header.flags & file_encrypted_flag) != 0) ||
		((header.flags & data_descriptor_flag) != 0) ||
		((header.flags & patched_data_flag) != 0) ||
		((header.flags & strong_encryption_flag) != 0) ||
		((header.flags & dir_encrypted_flag) != 0) ||
		header.has_extended_language_encoding_data ||
		header.has_info_zip_unicode_path_extra_field)
	{
		return false;
	}
	switch (header.compression_method)
	{
		case compression_method_store:
		case compression_method_deflate:
			break;
		default:
			return false;
	}
	if (header.compressed_size > max_data_size ||
		header.uncompressed_size > max_data_size)
	{
		return false;
	}
	if (header.file_name_length == 0)
		return false;
	if (header.local_header_offset == zip64_marker_32)
		return false;
	if (header.file_name[header.file_name_length - 1] == '/')
		return false;
	return true;
}

bool ZipArchiveFile::initialize_entry_name(const CentralFileHeader& header, ArchiveFileEntry& entry)
{
	BSTONE_ASSERT(header.file_name_length > 0);
	BSTONE_ASSERT(header.file_name != nullptr);
	const int aligned_size = align_16(header.file_name_length + 1);
	// The arena is sized from the central directory record, so this holds for any archive
	// the validation accepted. Keep it as a real check anyway: the names are copied
	// verbatim out of the archive and nothing else stands between them and the heap.
	if (names_capacity_ - names_size_ < aligned_size)
		return false;
	char* const entry_name = names_.get() + names_size_;
	names_size_ += aligned_size;
	entry.name = entry_name;
	std::copy_n(header.file_name, header.file_name_length, entry_name);
	entry_name[header.file_name_length] = '\0';
	entry.name = entry_name;
	return true;
}

void ZipArchiveFile::reserve_entries(int capacity)
{
	BSTONE_ASSERT(capacity >= 0);
	BSTONE_ASSERT(capacity <= INT_MAX / sizeof(ArchiveFileEntry));
	if (entries_capacity_ >= capacity)
		return;
	const int storage_size = capacity * sizeof(ArchiveFileEntry);
	Entries new_entries{static_cast<ArchiveFileEntry*>(::operator new(static_cast<std::size_t>(storage_size)))};
	entries_.swap(new_entries);
	entries_capacity_ = capacity;
}

bool ZipArchiveFile::add_entry(const CentralFileHeader& header)
{
	// The directory promises no more entries than were reserved, but the promise is the
	// archive's, so keep the bound where it holds in a release build too.
	if (entries_size_ >= entries_capacity_)
		return false;
	ArchiveFileEntry& entry = entries_[entries_size_];
	// Index.
	entry.index = entries_size_++;
	// Compression method.
	switch (header.compression_method)
	{
		case compression_method_store:
			entry.compression_method = ArchiveFileCompressionMethod::store;
			break;
		case compression_method_deflate:
			entry.compression_method = ArchiveFileCompressionMethod::deflate;
			break;
		default:
			entry.compression_method = ArchiveFileCompressionMethod::unknown;
			break;
	}
	// Is compressed?
	entry.is_compressed = (entry.compression_method != ArchiveFileCompressionMethod::store);
	// Name encoding.
	const bool is_utf8 = ((header.flags & language_flag) != 0);
	entry.language_encoding = is_utf8 ? ArchiveFileLanguageEncoding::utf8 : ArchiveFileLanguageEncoding::cp437;
	// Name length.
	entry.name_length = header.file_name_length;
	// Name.
	if (!initialize_entry_name(header, entry))
		return false;
	// Compressed size.
	entry.compressed_size = static_cast<int>(header.compressed_size);
	// Uncompressed size.
	entry.uncompressed_size = static_cast<int>(header.uncompressed_size);
	// Local file header position.
	entry.internal_file_position = header.local_header_offset;
	// CRC-32 type.
	entry.crc_32_type = ArchiveFileCrc32Type::zip;
	entry.crc_32 = header.crc_32;
	return true;
}

void ZipArchiveFile::reserve_names(int capacity)
{
	BSTONE_ASSERT(capacity >= 0);
	if (names_capacity_ >= capacity)
		return;
	Names new_names{static_cast<char*>(::operator new(static_cast<std::size_t>(capacity)))};
	names_.swap(new_names);
	names_capacity_ = capacity;
}

bool ZipArchiveFile::read_central_dir(const EndOfCentralDirRecord& eocdr)
{
	BufferUPtr dir_buffer{::operator new(std::size_t{eocdr.dir_size})};
	reserve_entries(eocdr.dir_total_entries);
	if (!input_stream_->set_position(eocdr.dir_offset))
		return false;
	MemoryBinaryReader dir_binary_reader{dir_buffer.get(), static_cast<int>(eocdr.dir_size)};
	if (!input_stream_->read_exactly(dir_buffer.get(), static_cast<int>(eocdr.dir_size)))
		return false;
	const int names_capacity = static_cast<int>(eocdr.dir_size) -
		((central_file_header_size - 16 /* (alignment-1)+NULL */) * eocdr.dir_total_entries);
	reserve_names(names_capacity);
	CentralFileHeader file_header;
	for (int i = 0; i < eocdr.dir_total_entries; ++i)
	{
		if (!deserialize_central_file_header(dir_binary_reader, file_header))
			return false;
		if (!validate_central_file_header(file_header))
			return false;
		if (is_central_file_supported(file_header) && !add_entry(file_header))
			return false;
	}
	return true;
}

bool ZipArchiveFile::prepare_input_stream_for_entry(ArchiveFileInputStream& input_stream, const ArchiveFileEntry& entry) const
{
	if (!input_stream.set_position(entry.internal_file_position))
		return false;
	unsigned char raw_local_file_header[local_file_header_size];
	if (!input_stream.read_exactly(raw_local_file_header, local_file_header_size))
		return false;
	MemoryBinaryReader binary_reader{&raw_local_file_header, local_file_header_size};
	LocalFileHeader local_file_header;
	deserialize_local_file_header(binary_reader, local_file_header);
	if (!validate_local_file_header(local_file_header, entry))
		return false;
	const int to_skip_size = local_file_header.file_name_length + local_file_header.extra_field_length;
	if (input_stream.skip(to_skip_size) < 0)
		return false;
	return true;
}

bool ZipArchiveFile::impl_open()
{
	if (!input_stream_->is_open())
		return false;
	EndOfCentralDirRecord eocdr;
	if (!read_end_of_central_dir_record(eocdr))
		return false;
	if (!validate_end_of_central_dir_record(eocdr))
		return false;
	if (!read_central_dir(eocdr))
		return false;
	is_open_ = true;
	return true;
}

void ZipArchiveFile::impl_close()
{
	is_open_ = false;
	input_stream_ = nullptr;
	names_size_ = 0;
	entries_capacity_ = 0;
	entries_size_ = 0;
}

bool ZipArchiveFile::impl_is_open() const
{
	return is_open_;
}

bool ZipArchiveFile::impl_open(const OpenParam& param)
{
	impl_close();
	if (param.is_file)
		input_stream_ = make_archive_file_file_input_stream(param.file.file_path);
	else
		input_stream_ = make_archive_file_memory_input_stream(param.memory.buffer, param.memory.size);
	if (!impl_open())
	{
		impl_close();
		return false;
	}
	return true;
}

int ZipArchiveFile::impl_get_entry_count() const
{
	return entries_size_;
}

} // namespace

// ======================================

ArchiveFileUPtr make_zip_archive_file()
{
	return std::make_unique<ZipArchiveFile>();
}

} // namespace bstone
