/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A binary reader for a block of memory.

#ifndef BSTONE_MEMORY_BINARY_READER_INCLUDED
#define BSTONE_MEMORY_BINARY_READER_INCLUDED

#include <cstdint>

namespace bstone {

class MemoryBinaryReader
{
public:
	MemoryBinaryReader() = default;
	MemoryBinaryReader(const void* data, int size);

	void set_position(int position);
	void skip(int count);
	bool can_read_n(int count) const;
	bool can_read_x8() const;
	bool can_read_x16() const;
	bool can_read_x32() const;
	std::int8_t read_s8();
	std::uint8_t read_u8();
	std::int16_t read_s16_le();
	std::uint16_t read_u16_le();
	std::int32_t read_s32_le();
	std::uint32_t read_u32_le();
	void read(void* buffer, int size);

private:
	const std::uint8_t* data_{};
	int size_{};
	int position_{};
};

} // namespace bstone

#endif // BSTONE_MEMORY_BINARY_READER_INCLUDED
