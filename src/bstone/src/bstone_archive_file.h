/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_ARCHIVE_FILE_INCLUDED
#define BSTONE_ARCHIVE_FILE_INCLUDED

#include "bstone_archive_file_entry_stream.h"
#include <memory>

namespace bstone {

enum class ArchiveFileCompressionMethod
{
	unknown = 0,
	store,
	deflate,
};

enum class ArchiveFileCrc32Type
{
	none = 0,
	zip,
};

enum class ArchiveFileLanguageEncoding
{
	unknown = 0,
	cp437,
	utf8,
};

// ======================================

struct ArchiveFileEntry
{
	int index; // Zero-based index of the entry.
	ArchiveFileCompressionMethod compression_method;
	ArchiveFileLanguageEncoding language_encoding;
	bool is_compressed; // If "true" the data is compressed or stored otherwise.
	const char* name; // Slash as separator; NULL-terminated.
	int name_length; // Excluding the NULL.
	int compressed_size;
	int uncompressed_size;
	long long internal_file_position;
	ArchiveFileCrc32Type crc_32_type;
	unsigned int crc_32;
};

// ======================================

class ArchiveFile
{
public:
	ArchiveFile() = default;
	virtual ~ArchiveFile() = default;

	virtual bool open_file(const char* file_path) = 0;
	virtual bool open_memory(const void* buffer, int size) = 0;
	virtual void close() = 0;

	virtual bool is_open() const = 0;
	virtual int get_entry_count() const = 0;
	virtual const ArchiveFileEntry& get_entry(int entry_index) const = 0;
	virtual ArchiveFileEntryStreamUPtr open_entry_stream(int entry_index) const = 0;
};

// ======================================

using ArchiveFileUPtr = std::unique_ptr<ArchiveFile>;

} // namespace bstone

#endif // BSTONE_ARCHIVE_FILE_INCLUDED
