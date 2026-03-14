/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// File stream.

#ifndef BSTONE_FILE_STREAM_INCLUDED
#define BSTONE_FILE_STREAM_INCLUDED

#include "bstone_stream.h"
#include "bstone_sys_file.h"

namespace bstone {

class FileStream final : public Stream
{
public:
	FileStream() = default;

	FileStream(const char* path, sys::FileMode mode);

	FileStream(FileStream&&) noexcept = default;
	FileStream& operator=(FileStream&&) noexcept = default;
	~FileStream() override = default;

	bool open(const char* path, sys::FileMode mode);

	void close() override;
	bool is_open() const override;
	std::intptr_t read(void* buffer, std::intptr_t count) override;
	std::intptr_t write(const void* buffer, std::intptr_t count) override;
	std::int64_t seek(std::int64_t offset, StreamOrigin origin) override;
	std::int64_t get_size() override;
	void set_size(std::int64_t size) override;
	void flush() override;

private:
	sys::File file_{};
};

} // namespace bstone

#endif // BSTONE_FILE_STREAM_INCLUDED
