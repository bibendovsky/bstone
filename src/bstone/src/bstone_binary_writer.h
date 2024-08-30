/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Writes primitive data types as binary values.

#ifndef BSTONE_BINARY_WRITER_INCLUDED
#define BSTONE_BINARY_WRITER_INCLUDED

#include <cstdint>

#include "bstone_stream.h"

namespace bstone {

class BinaryWriter
{
public:
	BinaryWriter() = delete;
	explicit BinaryWriter(Stream& stream);

	Stream& get_stream() const;

	void write_s8(std::int8_t value) const;
	void write_u8(std::uint8_t value) const;
	void write_s16(std::int16_t value) const;
	void write_u16(std::uint16_t value) const;
	void write_s32(std::int32_t value) const;
	void write_u32(std::uint32_t value) const;
	void write_s64(std::int64_t value) const;
	void write_u64(std::uint64_t value) const;
	void write_b32(float value) const; // IEEE 754-2008 binary32
	void write_b64(double value) const; // IEEE 754-2008 binary64

private:
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_BINARY_WRITER_INCLUDED
