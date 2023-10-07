/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Writes primitive data types as binary values.

#if !defined(BSTONE_BINARY_WRITER_INCLUDED)
#define BSTONE_BINARY_WRITER_INCLUDED

#include "bstone_int.h"
#include "bstone_stream.h"

namespace bstone {

class BinaryWriter
{
public:
	BinaryWriter() = delete;
	explicit BinaryWriter(Stream& stream);

	Stream& get_stream() const;

	void write_s8(Int8 value) const;
	void write_u8(UInt8 value) const;
	void write_s16(Int16 value) const;
	void write_u16(UInt16 value) const;
	void write_s32(Int32 value) const;
	void write_u32(UInt32 value) const;
	void write_s64(Int64 value) const;
	void write_u64(UInt64 value) const;
	void write_b32(float value) const; // IEEE 754-2008 binary32
	void write_b64(double value) const; // IEEE 754-2008 binary64

private:
	Stream* stream_{};
};

} // namespace bstone

#endif // BSTONE_BINARY_WRITER_INCLUDED
