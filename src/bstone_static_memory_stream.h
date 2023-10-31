/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Non-resizeable read-write memory stream.

#if !defined(BSTONE_STATIC_MEMORY_STREAM_INCLUDED)
#define BSTONE_STATIC_MEMORY_STREAM_INCLUDED

#include <cstdint>

#include "bstone_stream.h"

namespace bstone {

class StaticMemoryStream final : public Stream
{
public:
	StaticMemoryStream() = default;
	StaticMemoryStream(void* buffer, std::intptr_t size);
	~StaticMemoryStream() override = default;

	const std::uint8_t* get_data() const;
	std::uint8_t* get_data();

	void open(void* buffer, std::intptr_t size);

private:
	bool is_open_{};
	std::uint8_t* buffer_{};
	std::int64_t size_{};
	std::intptr_t capacity_{};
	std::int64_t position_{};

private:
	void do_close() override;
	bool do_is_open() const override;
	std::intptr_t do_read(void* buffer, std::intptr_t count) override;
	std::intptr_t do_write(const void* buffer, std::intptr_t count) override;
	std::int64_t do_seek(std::int64_t offset, StreamOrigin origin) override;
	std::int64_t do_get_size() const override;
	void do_set_size(std::int64_t size) override;
	void do_flush() override;

private:
	void ensure_is_open() const;
	void close_internal() noexcept;
};

} // namespace bstone

#endif // BSTONE_STATIC_MEMORY_STREAM_INCLUDED
