/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2026 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Endianness

#ifndef BSTONE_ENDIAN_INCLUDED
#define BSTONE_ENDIAN_INCLUDED

#include <cstdint>

namespace bstone::endian {

std::int16_t read_s16_le(const void* buffer);
std::uint16_t read_u16_le(const void* buffer);

std::int32_t read_s32_le(const void* buffer);
std::uint32_t read_u32_le(const void* buffer);

void write_s16_le(std::int16_t value, void* buffer);
void write_u16_le(std::uint16_t value, void* buffer);

void write_s32_le(std::int32_t value, void* buffer);
void write_u32_le(std::uint32_t value, void* buffer);

} // namespace bstone::endian

#endif // BSTONE_ENDIAN_INCLUDED
