/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bstone_archive_file.h"
#include "bstone_archive_file_input_stream.h"
#include "bstone_assert.h"
#include "bstone_zip_archive_file_crc_32.h"
#include <algorithm>

namespace bstone {

namespace {

class ArchiveFileStoreStream final : public ArchiveFileEntryStream
{
public:
	ArchiveFileStoreStream(ArchiveFileInputStreamUPtr&& input_stream_uptr, const ArchiveFileEntry& archive_file_entry);
	~ArchiveFileStoreStream() override = default;

	bool rewind() override;
	int get_size() override;
	int read(void* buffer, int count) override;

private:
	ArchiveFileInputStreamUPtr input_stream_uptr_{};
	std::int64_t begin_position_{};
	std::int64_t end_position_{};
	std::int64_t position_{};
	bool is_zip_crc32_{};
	unsigned int ref_crc_32_{};
	ZipArchiveFileCrc32 zip_crc_32_{};

private:
	bool impl_open(ArchiveFileInputStreamUPtr&& input_stream_uptr, const ArchiveFileEntry& archive_file_entry);
	void impl_close();
	bool impl_is_open() const;
};

// --------------------------------------

ArchiveFileStoreStream::ArchiveFileStoreStream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	if (!impl_open(std::move(input_stream_uptr), archive_file_entry))
	{
		impl_close();
	}
}

bool ArchiveFileStoreStream::rewind()
{
	BSTONE_ASSERT(impl_is_open());
	if (!input_stream_uptr_->set_position(begin_position_))
		return false;
	position_ = begin_position_;
	// The accumulator has to start over with the data, otherwise a rewind after a partial
	// read hashes that prefix twice and the entry fails its own checksum.
	if (is_zip_crc32_)
	{
		zip_crc_32_.reset();
	}
	return true;
}

int ArchiveFileStoreStream::get_size()
{
	BSTONE_ASSERT(impl_is_open());
	return static_cast<int>(end_position_ - begin_position_);
}

int ArchiveFileStoreStream::read(void* buffer, int count)
{
	BSTONE_ASSERT(impl_is_open());
	BSTONE_ASSERT(buffer != nullptr);
	BSTONE_ASSERT(count >= 0);
	const int to_read_size = std::min(static_cast<int>(end_position_ - position_), count);
	const int read_size = input_stream_uptr_->read(buffer, to_read_size);
	if (read_size > 0)
	{
		position_ += read_size;
		if (is_zip_crc32_ && !zip_crc_32_.is_finished())
			zip_crc_32_.update(buffer, read_size);
	}
	if (is_zip_crc32_ && position_ == end_position_)
	{
		if (!zip_crc_32_.is_finished())
			zip_crc_32_.finish();
		if (zip_crc_32_.get_value() != ref_crc_32_)
			return -1;
	}
	return read_size;
}

bool ArchiveFileStoreStream::impl_open(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	input_stream_uptr_.swap(input_stream_uptr);
	const long long data_position = input_stream_uptr_->skip(0);
	if (data_position < 0)
		return false;
	begin_position_ = data_position;
	end_position_ = begin_position_ + archive_file_entry.uncompressed_size;
	position_ = begin_position_;
	if (archive_file_entry.crc_32_type == ArchiveFileCrc32Type::zip)
	{
		is_zip_crc32_ = true;
		ref_crc_32_ = archive_file_entry.crc_32;
		zip_crc_32_.reset();
	}
	return true;
}

void ArchiveFileStoreStream::impl_close()
{
	input_stream_uptr_ = nullptr;
}

bool ArchiveFileStoreStream::impl_is_open() const
{
	return input_stream_uptr_ != nullptr && input_stream_uptr_->is_open();
}

} // namespace

// ======================================

ArchiveFileEntryStreamUPtr make_archive_file_store_entry_stream(
	ArchiveFileInputStreamUPtr&& input_stream_uptr,
	const ArchiveFileEntry& archive_file_entry)
{
	BSTONE_ASSERT(archive_file_entry.compression_method == ArchiveFileCompressionMethod::store);
	BSTONE_ASSERT(!archive_file_entry.is_compressed);
	BSTONE_ASSERT(archive_file_entry.compressed_size >= 0);
	BSTONE_ASSERT(archive_file_entry.uncompressed_size >= 0);
	BSTONE_ASSERT(archive_file_entry.compressed_size == archive_file_entry.uncompressed_size);
	return std::make_unique<ArchiveFileStoreStream>(std::move(input_stream_uptr), archive_file_entry);
}

} // namespace bstone
