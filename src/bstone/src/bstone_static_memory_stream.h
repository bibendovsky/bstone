/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Writable memory stream with external fixed-capacity storage.

#ifndef BSTONE_STATIC_MEMORY_STREAM_INCLUDED
#define BSTONE_STATIC_MEMORY_STREAM_INCLUDED

#include <cstdint>

#include "bstone_stream.h"

namespace bstone {

class StaticMemoryStream final : public Stream
{
public:
	StaticMemoryStream() noexcept;
	StaticMemoryStream(void* buffer, std::intptr_t buffer_size);
	~StaticMemoryStream() override;

	const std::uint8_t* get_data() const;
	std::uint8_t* get_data();

	void open(void* buffer, std::intptr_t buffer_size);

private:
	bool is_open_{};
	std::uint8_t* buffer_{};
	std::intptr_t size_{};
	std::intptr_t capacity_{};
	std::intptr_t position_{};

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
	void close_internal() noexcept;
};

} // namespace bstone

#endif // BSTONE_STATIC_MEMORY_STREAM_INCLUDED
