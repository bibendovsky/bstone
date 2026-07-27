/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include "bstone_memory_binary_reader.h"
#include "bstone_assert.h"
#include "bstone_endian.h"
#include <algorithm>

namespace bstone {

MemoryBinaryReader::MemoryBinaryReader(const void* data, int size)
	:
	data_{static_cast<const std::uint8_t*>(data)},
	size_{size}
{}

int MemoryBinaryReader::get_size() const
{
	return size_;
}

const void* MemoryBinaryReader::get_current_data() const
{
	return data_ + position_;
}

void MemoryBinaryReader::set_position(int position)
{
	position_ = std::clamp(position, 0, size_);
}

void MemoryBinaryReader::skip(int count)
{
	set_position(position_ + count);
}

bool MemoryBinaryReader::can_read_n(int count) const
{
	// Counts often come from the data being parsed. A negative one would sail
	// through the comparison below and vouch for a read that can not be made.
	return count >= 0 && size_ - position_ >= count;
}

bool MemoryBinaryReader::can_read_x8() const
{
	return can_read_n(1);
}

bool MemoryBinaryReader::can_read_x16() const
{
	return can_read_n(2);
}

bool MemoryBinaryReader::can_read_x32() const
{
	return can_read_n(4);
}

std::int8_t MemoryBinaryReader::read_s8()
{
	return static_cast<std::int8_t>(read_u8());
}

std::uint8_t MemoryBinaryReader::read_u8()
{
	BSTONE_ASSERT(can_read_x8());
	return static_cast<std::uint8_t>(data_[position_++]);
}

std::int16_t MemoryBinaryReader::read_s16_le()
{
	return static_cast<std::int16_t>(read_u16_le());
}

std::uint16_t MemoryBinaryReader::read_u16_le()
{
	BSTONE_ASSERT(can_read_x16());
	const std::uint16_t value = endian::read_u16_le(data_ + position_);
	position_ += 2;
	return value;
}

std::int32_t MemoryBinaryReader::read_s32_le()
{
	return static_cast<std::int32_t>(read_u32_le());
}

std::uint32_t MemoryBinaryReader::read_u32_le()
{
	BSTONE_ASSERT(can_read_x32());
	const std::uint32_t value = endian::read_u32_le(data_ + position_);
	position_ += 4;
	return value;
}

void MemoryBinaryReader::read(void* buffer, int size)
{
	BSTONE_ASSERT(can_read_n(size));
	std::copy_n(data_ + position_, size, static_cast<std::uint8_t*>(buffer));
	position_ += size;
}

} // namespace bstone
