/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BSTONE_ARCHIVE_FILE_ENTRY_STREAM_INCLUDED
#define BSTONE_ARCHIVE_FILE_ENTRY_STREAM_INCLUDED

#include <memory>

namespace bstone {

class ArchiveFileEntryStream
{
public:
	ArchiveFileEntryStream() = default;
	virtual ~ArchiveFileEntryStream() = default;

	virtual bool rewind() = 0;
	// Returns negative value on error.
	virtual int get_size() = 0;
	// Returns negative value on error.
	virtual int read(void* buffer, int count) = 0;
};

// ======================================

using ArchiveFileEntryStreamUPtr = std::unique_ptr<ArchiveFileEntryStream>;

} // namespace bstone

#endif // BSTONE_ARCHIVE_FILE_ENTRY_STREAM_INCLUDED
