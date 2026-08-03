/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2023-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Endianness

#include "bstone_endian.h"
#include <bit>
#include <limits>

namespace bstone::endian {

static_assert(std::numeric_limits<float>::is_iec559, "Requires IEC 559 binary32 data type.");

// =====================================

std::int16_t read_s16_le(const void* buffer)
{
	return static_cast<std::int16_t>(read_u16_le(buffer));
}

std::uint16_t read_u16_le(const void* buffer)
{
	const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(buffer);
	return static_cast<std::uint16_t>(
		 static_cast<std::uint_fast16_t>(bytes[0])     ) |
		(static_cast<std::uint_fast16_t>(bytes[1]) << 8);
}

std::int32_t read_s32_le(const void* buffer)
{
	return static_cast<std::int32_t>(read_u32_le(buffer));
}

std::uint32_t read_u32_le(const void* buffer)
{
	const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(buffer);
	return static_cast<std::uint32_t>(
		 static_cast<std::uint_fast32_t>(bytes[0])      ) |
		(static_cast<std::uint_fast32_t>(bytes[1]) <<  8) |
		(static_cast<std::uint_fast32_t>(bytes[2]) << 16) |
		(static_cast<std::uint_fast32_t>(bytes[3]) << 24);
}

std::uint32_t read_u32_be(const void* buffer)
{
	const std::uint8_t* const bytes = static_cast<const std::uint8_t*>(buffer);
	return static_cast<std::uint32_t>(
		(static_cast<std::uint_fast32_t>(bytes[0]) << 24) |
		(static_cast<std::uint_fast32_t>(bytes[1]) << 16) |
		(static_cast<std::uint_fast32_t>(bytes[2]) <<  8) |
		 static_cast<std::uint_fast32_t>(bytes[3])       );
}

float read_f32_le(const void* buffer)
{
	return std::bit_cast<float>(read_u32_le(buffer));
}

void write_s16_le(std::int16_t value, void* buffer)
{
	write_u16_le(static_cast<std::uint16_t>(value), buffer);
}

void write_u16_le(std::uint16_t value, void* buffer)
{
	std::uint8_t* const bytes = static_cast<std::uint8_t*>(buffer);
	bytes[0] = static_cast<std::uint8_t>( static_cast<std::uint_fast16_t>(value)       & 0xFF);
	bytes[1] = static_cast<std::uint8_t>((static_cast<std::uint_fast16_t>(value) >> 8) & 0xFF);
}

void write_s32_le(std::int32_t value, void* buffer)
{
	write_u32_le(static_cast<std::uint32_t>(value), buffer);
}

void write_u32_le(std::uint32_t value, void* buffer)
{
	std::uint8_t* const bytes = static_cast<std::uint8_t*>(buffer);
	bytes[0] = static_cast<std::uint8_t>( static_cast<std::uint_fast32_t>(value)        & 0xFF);
	bytes[1] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >> 8 ) & 0xFF);
	bytes[2] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >> 16) & 0xFF);
	bytes[3] = static_cast<std::uint8_t>((static_cast<std::uint_fast32_t>(value) >> 24) & 0xFF);
}

} // namespace bstone::endian
