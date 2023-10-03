/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Stream primitive.

#if !defined(BSTONE_STREAM_INCLUDED)
#define BSTONE_STREAM_INCLUDED

#include "bstone_int.h"

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
	IntP read(void* buffer, IntP count);
	void read_exact(void* buffer, IntP count);
	IntP write(const void* buffer, IntP count);
	void write_exact(const void* buffer, IntP count);
	Int64 seek(Int64 offset, StreamOrigin origin);
	Int64 skip(Int64 delta);
	Int64 get_position();
	void set_position(Int64 position);
	Int64 get_size() const;
	void set_size(Int64 size);
	void flush();

private:
	virtual void do_close() = 0;
	virtual bool do_is_open() const = 0;
	virtual IntP do_read(void* buffer, IntP count) = 0;
	virtual IntP do_write(const void* buffer, IntP count) = 0;
	virtual Int64 do_seek(Int64 offset, StreamOrigin origin) = 0;
	virtual Int64 do_get_size() const = 0;
	virtual void do_set_size(Int64 size) = 0;
	virtual void do_flush() = 0;
};

} // namespace bstone

#endif // BSTONE_STREAM_INCLUDED
