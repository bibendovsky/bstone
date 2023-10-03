/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File stream.

#if !defined(BSTONE_FILE_STREAM_INCLUDED)
#define BSTONE_FILE_STREAM_INCLUDED

#include "bstone_file.h"
#include "bstone_stream.h"

namespace bstone {

class FileStream final : public Stream
{
public:
	FileStream() = default;
	explicit FileStream(const char* file_name, FileOpenMode open_mode = FileOpenMode::read);
	FileStream(FileStream&&) = default;
	FileStream& operator=(FileStream&&) = default;
	~FileStream() override = default;

	void open(const char* file_name, FileOpenMode open_mode = FileOpenMode::read);

private:
	File file_{};

private:
	void do_close() override;
	bool do_is_open() const override;
	IntP do_read(void* buffer, IntP count) override;
	IntP do_write(const void* buffer, IntP count) override;
	Int64 do_seek(Int64 offset, StreamOrigin origin) override;
	Int64 do_get_size() const override;
	void do_set_size(Int64 size) override;
	void do_flush() override;
};

} // namespace bstone

#endif // BSTONE_FILE_STREAM_INCLUDED
