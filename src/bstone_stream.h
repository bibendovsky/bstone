/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Stream primitive.

#ifndef BSTONE_STREAM_INCLUDED
#define BSTONE_STREAM_INCLUDED

#include <cstdint>

namespace bstone {

enum class StreamOrigin
{
	none,
	begin,
	current,
	end,
};

class Stream
{
public:
	Stream() noexcept;
	virtual ~Stream();

	void close() noexcept;
	bool is_open() const noexcept;
	std::intptr_t read(void* buffer, std::intptr_t count);
	void read_exactly(void* buffer, std::intptr_t count);
	std::intptr_t write(const void* buffer, std::intptr_t count);
	void write_exactly(const void* buffer, std::intptr_t count);
	std::int64_t seek(std::int64_t offset, StreamOrigin origin);
	std::int64_t skip(std::int64_t offset);
	std::int64_t get_position();
	void set_position(std::int64_t position);
	std::int64_t get_size();
	void set_size(std::int64_t size);
	void flush();

private:
	virtual void do_close() noexcept = 0;
	virtual bool do_is_open() const noexcept = 0;
	virtual std::intptr_t do_read(void* buffer, std::intptr_t count) = 0;
	virtual std::intptr_t do_write(const void* buffer, std::intptr_t count) = 0;
	virtual std::int64_t do_seek(std::int64_t offset, StreamOrigin origin) = 0;
	virtual std::int64_t do_get_size() = 0;
	virtual void do_set_size(std::int64_t size) = 0;
	virtual void do_flush() = 0;
};

} // namespace bstone

#endif // BSTONE_STREAM_INCLUDED
