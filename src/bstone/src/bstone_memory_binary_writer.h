/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A binary writer for a block of memory.

#ifndef BSTONE_MEMORY_BINARY_WRITER_INCLUDED
#define BSTONE_MEMORY_BINARY_WRITER_INCLUDED

#include <cstdint>

namespace bstone {

class MemoryBinaryWriter
{
public:
	MemoryBinaryWriter() = default;
	MemoryBinaryWriter(void* data, int capacity);

	bool can_write_n(int size) const;
	bool can_write_x8() const;
	bool can_write_x16() const;
	bool can_write_x32() const;
	void write_s8(std::int8_t value);
	void write_u8(std::uint8_t value);
	void write_s16_le(std::int16_t value);
	void write_u16_le(std::uint16_t value);
	void write_s32_le(std::int32_t value);
	void write_u32_le(std::uint32_t value);
	void write(const void* buffer, int size);

private:
	std::uint8_t* data_{};
	int capacity_{};
	int position_{};
};

} // namespace bstone

#endif // BSTONE_MEMORY_BINARY_WRITER_INCLUDED
