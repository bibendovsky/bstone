/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Resizable memory stream.

#if !defined(BSTONE_MEMORY_STREAM_INCLUDED)
#define BSTONE_MEMORY_STREAM_INCLUDED

#include <memory>

#include "bstone_int.h"
#include "bstone_stream.h"

namespace bstone {

class MemoryStream final : public Stream
{
public:
	static constexpr auto default_initial_capacity = 4096;
	static constexpr auto default_chunk_size = 4096;

public:
	MemoryStream() = default;
	explicit MemoryStream(IntP initial_capacity, IntP chunk_size = default_chunk_size);
	MemoryStream(MemoryStream&&) = default;
	MemoryStream& operator=(MemoryStream&&) = default;
	~MemoryStream() override = default;

	const UInt8* get_data() const;
	UInt8* get_data();

	void open(IntP initial_capacity = default_initial_capacity, IntP chunk_size = default_chunk_size);

private:
	using Storage = std::unique_ptr<UInt8[]>;

private:
	bool is_open_{};
	IntP capacity_{};
	IntP chunk_size_{};
	IntP size_{};
	IntP position_{};
	Storage storage_{};

private:
	void do_close() override;
	bool do_is_open() const override;
	IntP do_read(void* buffer, IntP count) override;
	IntP do_write(const void* buffer, IntP count) override;
	Int64 do_seek(Int64 offset, StreamOrigin origin) override;
	Int64 do_get_size() const override;
	void do_set_size(Int64 size) override;
	void do_flush() override;

private:
	void ensure_is_open() const;
	void reserve(IntP capacity, IntP chunk_size);
	void close_internal();
};

} // namespace bstone

#endif // BSTONE_MEMORY_STREAM_INCLUDED
