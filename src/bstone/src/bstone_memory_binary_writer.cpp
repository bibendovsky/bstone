/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// A binary writer for a block of memory.

#include "bstone_memory_binary_writer.h"
#include "bstone_assert.h"
#include "bstone_endian.h"
#include <algorithm>

namespace bstone {

MemoryBinaryWriter::MemoryBinaryWriter(void* data, int capacity)
	:
	data_{static_cast<std::uint8_t*>(data)},
	capacity_{capacity}
{}

bool MemoryBinaryWriter::can_write_n(int size) const
{
	return capacity_ - position_ >= size;
}

bool MemoryBinaryWriter::can_write_x8() const
{
	return can_write_n(1);
}

bool MemoryBinaryWriter::can_write_x16() const
{
	return can_write_n(2);
}

bool MemoryBinaryWriter::can_write_x32() const
{
	return can_write_n(4);
}

void MemoryBinaryWriter::write_s8(std::int8_t value)
{
	write_u8(static_cast<std::uint8_t>(value));
}

void MemoryBinaryWriter::write_u8(std::uint8_t value)
{
	BSTONE_ASSERT(can_write_x8());
	data_[position_++] = value;
}

void MemoryBinaryWriter::write_s16_le(std::int16_t value)
{
	write_u16_le(static_cast<std::uint16_t>(value));
}

void MemoryBinaryWriter::write_u16_le(std::uint16_t value)
{
	BSTONE_ASSERT(can_write_x16());
	endian::write_u16_le(value, data_ + position_);
	position_ += 2;
}

void MemoryBinaryWriter::write_s32_le(std::int32_t value)
{
	write_u32_le(static_cast<std::uint32_t>(value));
}

void MemoryBinaryWriter::write_u32_le(std::uint32_t value)
{
	BSTONE_ASSERT(can_write_x32());
	endian::write_u32_le(value, data_ + position_);
	position_ += 4;
}

void MemoryBinaryWriter::write(const void* buffer, int size)
{
	BSTONE_ASSERT(can_write_n(size));
	std::copy_n(static_cast<const std::uint8_t*>(buffer), size, data_ + position_);
	position_ += size;
}

} // namespace bstone
