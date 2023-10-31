/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Stream primitive.

#if !defined(BSTONE_STREAM_INCLUDED)
#define BSTONE_STREAM_INCLUDED

#include <cstdint>

namespace bstone {

enum class StreamOrigin
{
	begin,
	current,
	end,
};

class Stream
{
public:
	Stream() = default;
	virtual ~Stream() = default;

	void close();
	bool is_open() const;
	std::intptr_t read(void* buffer, std::intptr_t count);
	void read_exact(void* buffer, std::intptr_t count);
	std::intptr_t write(const void* buffer, std::intptr_t count);
	void write_exact(const void* buffer, std::intptr_t count);
	std::int64_t seek(std::int64_t offset, StreamOrigin origin);
	std::int64_t skip(std::int64_t delta);
	std::int64_t get_position();
	void set_position(std::int64_t position);
	std::int64_t get_size() const;
	void set_size(std::int64_t size);
	void flush();

private:
	virtual void do_close() = 0;
	virtual bool do_is_open() const = 0;
	virtual std::intptr_t do_read(void* buffer, std::intptr_t count) = 0;
	virtual std::intptr_t do_write(const void* buffer, std::intptr_t count) = 0;
	virtual std::int64_t do_seek(std::int64_t offset, StreamOrigin origin) = 0;
	virtual std::int64_t do_get_size() const = 0;
	virtual void do_set_size(std::int64_t size) = 0;
	virtual void do_flush() = 0;
};

} // namespace bstone

#endif // BSTONE_STREAM_INCLUDED
