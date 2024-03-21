/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Resizable memory stream.

#ifndef BSTONE_MEMORY_STREAM_INCLUDED
#define BSTONE_MEMORY_STREAM_INCLUDED

#include <cstdint>

#include <memory>

#include "bstone_stream.h"

namespace bstone {

class MemoryStream final : public Stream
{
public:
	static constexpr auto default_capacity = 4096;
	static constexpr auto default_chunk_size = 4096;

public:
	MemoryStream() noexcept = default;
	explicit MemoryStream(
		std::intptr_t capacity,
		std::intptr_t chunk_size = default_chunk_size);
	MemoryStream(MemoryStream&&) noexcept = default;
	MemoryStream& operator=(MemoryStream&&) noexcept = default;
	~MemoryStream() override = default;

	const std::uint8_t* get_data() const;
	std::uint8_t* get_data();

	void open(
		std::intptr_t capacity = default_capacity,
		std::intptr_t chunk_size = default_chunk_size);

private:
	using Storage = std::unique_ptr<std::uint8_t[]>;

private:
	bool is_open_{};
	std::intptr_t capacity_{};
	std::intptr_t chunk_size_{};
	std::intptr_t size_{};
	std::intptr_t position_{};
	Storage storage_{};

private:
	void do_close() noexcept override;
	bool do_is_open() const noexcept override;
	std::intptr_t do_read(void* buffer, std::intptr_t count) override;
	std::intptr_t do_write(const void* buffer, std::intptr_t count) override;
	std::int64_t do_seek(std::int64_t offset, StreamOrigin origin) override;
	std::int64_t do_get_size() override;
	void do_set_size(std::int64_t size) override;
	void do_flush() override;

private:
	void ensure_is_open() const;
	static void validate_capacity(std::intptr_t capacity);
	static void validate_chunk_size(std::intptr_t chunk_size);
	static void validate_buffer(const void* buffer);
	static void validate_count(std::intptr_t count);
	static void validate_offset(std::int64_t offset);
	static void validate_size(std::int64_t size);

	void reserve(std::intptr_t capacity, std::intptr_t chunk_size);
	void close_internal() noexcept;
};

} // namespace bstone

#endif // BSTONE_MEMORY_STREAM_INCLUDED
