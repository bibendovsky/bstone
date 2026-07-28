/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_archive_file.h"
#include "bstone_archive_file_input_stream.h"
#include "bstone_assert.h"
#include "bstone_zip_archive_file_crc_32.h"
#include <climits>
#include <algorithm>
#include <new>
#include "zlib.h"

namespace bstone {

namespace {

class ArchiveFileInflateStream final : public ArchiveFileEntryStream
{
public:
	ArchiveFileInflateStream(ArchiveFileInputStreamUPtr&& input_stream_uptr, const ArchiveFileEntry& archive_file_entry);
	~ArchiveFileInflateStream() override = default;

	bool rewind() override;
	int get_size() override;
	int read(void* buffer, int count) override;

private:
	static constexpr int zlib_in_cache_max_capacity = 1024;
	static constexpr int zlib_out_cache_max_capacity = 2048;

private:
	using ZlibInCache = ::Bytef[zlib_in_cache_max_capacity];
	using ZlibOutCache = ::Bytef[zlib_out_cache_max_capacity];

private:
	ArchiveFileInputStreamUPtr input_stream_{};
	std::int64_t compressed_begin_position_{};
	std::int64_t compressed_end_position_{};
	std::int64_t compressed_position_{};
	int uncompressed_size_{};
	int uncompressed_position_{};
	::z_stream zlib_stream_{};
	ZlibInCache zlib_in_cache_{};
	ZlibOutCache zlib_out_cache_{};
	::uLong zlib_out_cache_offset_{};
	unsigned int crc_32_{};
	ZipArchiveFileCrc32 zlib_crc_32_{};

private:
	static ::voidpf zlib_alloc_func(::voidpf opaque, ::uInt items, ::uInt size);
	static void zlib_free_func(::voidpf opaque, ::voidpf address);

	bool initialize_zlib_stream();
	bool impl_open(ArchiveFileInputStreamUPtr&& input_stream_uptr, const ArchiveFileEntry& archive_file_entry);
	void impl_close();
	bool impl_is_open() const;
};

// --------------------------------------

ArchiveFileInflateStream::ArchiveFileInflateStream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	if (!impl_open(std::move(input_stream_uptr), archive_file_entry))
		impl_close();
}

bool ArchiveFileInflateStream::rewind()
{
	BSTONE_ASSERT(impl_is_open());
	if (!input_stream_->set_position(compressed_begin_position_))
		return false;
	if (::inflateReset2(&zlib_stream_, -15) != Z_OK)
		return false;
	// The reset leaves the input side to the caller, so any bytes still buffered from
	// before the rewind would be handed to the fresh inflate state as if they were the
	// beginning of the stream.
	zlib_stream_.next_in = zlib_in_cache_;
	zlib_stream_.avail_in = 0;
	compressed_position_ = compressed_begin_position_;
	uncompressed_position_ = 0;
	zlib_out_cache_offset_ = 0;
	zlib_crc_32_.reset();
	return true;
}

int ArchiveFileInflateStream::get_size()
{
	BSTONE_ASSERT(impl_is_open());
	return uncompressed_size_;
}

int ArchiveFileInflateStream::read(void* buffer, int count)
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);
	count = std::min(count, uncompressed_size_ - uncompressed_position_);
	::Bytef* dst_bytes = static_cast<::Bytef*>(buffer);
	bool is_stream_end = false;
	while (count > 0)
	{
		if (zlib_out_cache_offset_ < zlib_stream_.total_out)
		{
			const int write_count = std::min(count, static_cast<int>(zlib_stream_.total_out - zlib_out_cache_offset_));
			std::copy_n(zlib_out_cache_ + zlib_out_cache_offset_, write_count, dst_bytes);
			if (!zlib_crc_32_.is_finished())
				zlib_crc_32_.update(&zlib_out_cache_[zlib_out_cache_offset_], write_count);
			count -= write_count;
			dst_bytes += write_count;
			zlib_out_cache_offset_ += write_count;
			uncompressed_position_ += write_count;
			continue;
		}
		else
		{
			// The cache is drained. Once inflate is done it keeps reporting the end of the
			// stream without producing anything, so an entry that decodes to less than its
			// declared uncompressed size would spin here forever.
			if (is_stream_end)
				break;
			zlib_out_cache_offset_ = 0;
			zlib_stream_.next_out = zlib_out_cache_;
			zlib_stream_.avail_out = zlib_out_cache_max_capacity;
			zlib_stream_.total_out = 0;
		}
		if (zlib_stream_.avail_in == 0)
		{
			const std::int64_t data_left = compressed_end_position_ - compressed_position_;
			const int to_read_count = std::min(static_cast<int>(data_left), zlib_in_cache_max_capacity);
			if (to_read_count > 0)
			{
				const int read_count = static_cast<int>(input_stream_->read(zlib_in_cache_, to_read_count));
				if (read_count < 0)
					return -1;
				if (read_count == 0)
					break;
				compressed_position_ += read_count;
				zlib_stream_.next_in = zlib_in_cache_;
				zlib_stream_.avail_in = static_cast<::uInt>(read_count);
				zlib_stream_.total_in = 0;
			}
		}
		const int zlib_result = ::inflate(&zlib_stream_, Z_NO_FLUSH);
		switch (zlib_result)
		{
			case Z_OK:
				break;
			case Z_STREAM_END:
				is_stream_end = true;
				break;
			case Z_NEED_DICT:
			case Z_ERRNO:
			case Z_STREAM_ERROR:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_BUF_ERROR:
			case Z_VERSION_ERROR:
			default:
				return -1;
		}
	}
	if (uncompressed_position_ == uncompressed_size_)
	{
		if (!zlib_crc_32_.is_finished())
			zlib_crc_32_.finish();
		if (zlib_crc_32_.get_value() != crc_32_)
			return -1;
	}
	return static_cast<int>(dst_bytes - static_cast<::Bytef*>(buffer));
}

::voidpf ArchiveFileInflateStream::zlib_alloc_func([[maybe_unused]] ::voidpf opaque, ::uInt items, ::uInt size)
{
	if (SIZE_MAX / size < items)
		return nullptr;
	return ::operator new(std::size_t{items} * size, std::nothrow);
}

void ArchiveFileInflateStream::zlib_free_func([[maybe_unused]] ::voidpf opaque, ::voidpf address)
{
	::operator delete(address);
}

bool ArchiveFileInflateStream::initialize_zlib_stream()
{
	zlib_stream_.next_in = zlib_in_cache_;
	zlib_stream_.avail_in = 0;
	zlib_stream_.total_in = 0;
	zlib_stream_.next_out = zlib_out_cache_;
	zlib_stream_.avail_out = 0;
	zlib_stream_.total_out = 0;
	zlib_stream_.zalloc = &ArchiveFileInflateStream::zlib_alloc_func;
	zlib_stream_.zfree = &ArchiveFileInflateStream::zlib_free_func;
	zlib_stream_.opaque = nullptr;
	if (::inflateInit2(&zlib_stream_, -15) != Z_OK)
	{
		BSTONE_ASSERT(false && "Failed to initialize the inflate.");
		return false;
	}
	return true;
}

bool ArchiveFileInflateStream::impl_open(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	input_stream_ = std::move(input_stream_uptr);
	const long long data_position = input_stream_->skip(0);
	if (data_position < 0)
		return false;
	if (!initialize_zlib_stream())
		return false;
	compressed_begin_position_ = data_position;
	compressed_end_position_ = compressed_begin_position_ + archive_file_entry.compressed_size;
	compressed_position_ = compressed_begin_position_;
	uncompressed_size_ = archive_file_entry.uncompressed_size;
	uncompressed_position_ = 0;
	crc_32_ = archive_file_entry.crc_32;
	zlib_crc_32_.reset();
	return true;
}

void ArchiveFileInflateStream::impl_close()
{
	input_stream_ = nullptr;
}

bool ArchiveFileInflateStream::impl_is_open() const
{
	return input_stream_ != nullptr && input_stream_->is_open();
}

} // namespace

// ======================================

ArchiveFileEntryStreamUPtr make_archive_file_inflate_entry_stream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	BSTONE_ASSERT(archive_file_entry.compression_method == ArchiveFileCompressionMethod::deflate);
	BSTONE_ASSERT(archive_file_entry.is_compressed);
	BSTONE_ASSERT(archive_file_entry.compressed_size > 0);
	BSTONE_ASSERT(archive_file_entry.uncompressed_size >= 0);
	return std::make_unique<ArchiveFileInflateStream>(std::move(input_stream_uptr), archive_file_entry);
}

} // namespace bstone
