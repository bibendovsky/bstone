/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Reads primitive data types as binary values.

#if !defined(BSTONE_BINARY_READER_INCLUDED)
#define BSTONE_BINARY_READER_INCLUDED

#include "bstone_int.h"
#include "bstone_stream.h"

namespace bstone {

class BinaryReader
{
public:
	BinaryReader() = delete;
	explicit BinaryReader(Stream& stream);

	Stream& get_stream() const;

	Int8 read_s8() const;
	UInt8 read_u8() const;

	Int16 read_s16() const;
	UInt16 read_u16() const;

	Int32 read_s32() const;
	UInt32 read_u32() const;

	Int64 read_s64() const;
	UInt64 read_u64() const;

	float read_b32() const; // IEEE 754-2008 binary32
	double read_b64() const; // IEEE 754-2008 binary64

private:
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_BINARY_READER_INCLUDED
