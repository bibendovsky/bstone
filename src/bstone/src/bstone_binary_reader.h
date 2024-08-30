/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Reads primitive data types as binary values.

#ifndef BSTONE_BINARY_READER_INCLUDED
#define BSTONE_BINARY_READER_INCLUDED

#include <cstdint>

#include "bstone_stream.h"

namespace bstone {

class BinaryReader
{
public:
	BinaryReader() = delete;
	explicit BinaryReader(Stream& stream);

	Stream& get_stream() const;

	std::int8_t read_s8() const;
	std::uint8_t read_u8() const;

	std::int16_t read_s16() const;
	std::uint16_t read_u16() const;

	std::int32_t read_s32() const;
	std::uint32_t read_u32() const;

	std::int64_t read_s64() const;
	std::uint64_t read_u64() const;

	float read_b32() const; // IEEE 754-2008 binary32
	double read_b64() const; // IEEE 754-2008 binary64

private:
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_BINARY_READER_INCLUDED
