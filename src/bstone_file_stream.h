/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com)
SPDX-License-Identifier: GPL-2.0-or-later
*/

//
// File stream.
//

#ifndef BSTONE_FILE_STREAM_INCLUDED
#define BSTONE_FILE_STREAM_INCLUDED

#include "bstone_stream.h"

namespace bstone
{

// File stream.
class FileStream final : public Stream
{
public:
	FileStream() noexcept;
	FileStream(const std::string& file_name, StreamOpenMode open_mode = StreamOpenMode::read) noexcept;
	FileStream(const FileStream& rhs) = delete;
	FileStream(FileStream&& rhs) noexcept;
	FileStream& operator=(const FileStream& rhs) = delete;
	~FileStream() override;

	bool open(const std::string& file_name, StreamOpenMode open_mode = StreamOpenMode::read) noexcept;
	void close() noexcept override;
	bool is_open() const noexcept override;
	int get_size() noexcept override;
	bool set_size(int size) noexcept override;
	int seek(int offset, StreamSeekOrigin origin) noexcept override;
	int read(void* buffer, int count) noexcept override;
	bool write(const void* buffer, int count) noexcept override;
	bool flush() noexcept override;
	bool is_readable() const noexcept override;
	bool is_seekable() const noexcept override;
	bool is_writable() const noexcept override;

	static bool is_exists(const std::string& file_name) noexcept;

private:
#ifdef _WIN32
	void* handle_{};
#else
	int handle_{};
#endif // _WIN32

	bool is_readable_{};
	bool is_seekable_{};
	bool is_writable_{};

	bool is_open_internal() const noexcept;
	void close_handle() noexcept;
	void close_internal() noexcept;
}; // FileStream

} // bstone

#endif // BSTONE_FILE_STREAM_INCLUDED
