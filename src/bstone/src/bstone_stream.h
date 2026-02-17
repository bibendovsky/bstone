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
	Stream() = default;
	virtual ~Stream() = default;

	virtual void close() = 0;
	virtual bool is_open() const = 0;
	virtual std::intptr_t read(void* buffer, std::intptr_t count) = 0;
	virtual std::intptr_t write(const void* buffer, std::intptr_t count) = 0;
	virtual std::int64_t seek(std::int64_t offset, StreamOrigin origin) = 0;
	virtual std::int64_t get_size() = 0;
	virtual void set_size(std::int64_t size) = 0;
	virtual void flush() = 0;

	void read_exactly(void* buffer, std::intptr_t count);
	void write_exactly(const void* buffer, std::intptr_t count);
	std::int64_t skip(std::int64_t offset);
	std::int64_t get_position();
	void set_position(std::int64_t position);
};

} // namespace bstone

#endif // BSTONE_STREAM_INCLUDED
