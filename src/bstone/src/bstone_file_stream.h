/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File stream.

#ifndef BSTONE_FILE_STREAM_INCLUDED
#define BSTONE_FILE_STREAM_INCLUDED

#include "bstone_file.h"
#include "bstone_stream.h"

namespace bstone {

class FileStream final : public Stream
{
public:
	FileStream() noexcept;

	explicit FileStream(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

	FileStream(FileStream&&) noexcept;
	FileStream& operator=(FileStream&&) noexcept;
	~FileStream() override;

	bool try_open(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

	void open(
		const char* path,
		FileOpenFlags open_flags = FileOpenFlags::read,
		FileShareMode share_mode = FileShareMode::shared);

private:
	File file_{};

private:
	void do_close() noexcept override;
	bool do_is_open() const noexcept override;
	std::intptr_t do_read(void* buffer, std::intptr_t count) override;
	std::intptr_t do_write(const void* buffer, std::intptr_t count) override;
	std::int64_t do_seek(std::int64_t offset, StreamOrigin origin) override;
	std::int64_t do_get_size() override;
	void do_set_size(std::int64_t size) override;
	void do_flush() override;
};

} // namespace bstone

#endif // BSTONE_FILE_STREAM_INCLUDED
