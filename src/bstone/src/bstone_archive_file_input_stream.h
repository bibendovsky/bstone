/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_ARCHIVE_FILE_INPUT_STREAM_INCLUDED
#define BSTONE_ARCHIVE_FILE_INPUT_STREAM_INCLUDED

#include <cstdint>
#include <memory>

namespace bstone {

class ArchiveFileInputStream
{
public:
	ArchiveFileInputStream() = default;
	virtual ~ArchiveFileInputStream() = default;

	virtual ArchiveFileInputStream* open_from_this() = 0;
	virtual bool is_open() const = 0;
	virtual std::int64_t get_size() = 0;
	virtual bool set_position(std::int64_t position) = 0;
	virtual std::int64_t skip(std::int64_t count) = 0;
	virtual int read(void* buffer, int count) = 0;
	virtual bool read_exactly(void* buffer, int count) = 0;
};

// ======================================

using ArchiveFileInputStreamUPtr = std::unique_ptr<ArchiveFileInputStream>;

// ======================================

ArchiveFileInputStreamUPtr make_archive_file_file_input_stream(const char* file_path);
ArchiveFileInputStreamUPtr make_archive_file_memory_input_stream(const void* buffer, int count);

} // namespace bstone

#endif // BSTONE_ARCHIVE_FILE_INPUT_STREAM_INCLUDED
