/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2023 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Reads primitive data types as binary values.

#include <limits>

#include "bstone_binary_reader.h"

static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4, "Unsupported `float` type.");
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8, "Unsupported `double` type.");

namespace bstone {

namespace {

struct BinaryReaderDetail
{
	template<typename T>
	static T generic_read(Stream& stream)
	{
		T value;
		stream.read_exact(&value, static_cast<IntP>(sizeof(T)));
		return value;
	}
};

} // namespace

// ==========================================================================

BinaryReader::BinaryReader(Stream& stream)
	:
	stream_{&stream}
{}

Stream& BinaryReader::get_stream() const
{
	return *stream_;
}

Int8 BinaryReader::read_s8() const
{
	return BinaryReaderDetail::generic_read<Int8>(*stream_);
}

UInt8 BinaryReader::read_u8() const
{
	return BinaryReaderDetail::generic_read<UInt8>(*stream_);
}

Int16 BinaryReader::read_s16() const
{
	return BinaryReaderDetail::generic_read<Int16>(*stream_);
}

UInt16 BinaryReader::read_u16() const
{
	return BinaryReaderDetail::generic_read<UInt16>(*stream_);
}

Int32 BinaryReader::read_s32() const
{
	return BinaryReaderDetail::generic_read<Int32>(*stream_);
}

UInt32 BinaryReader::read_u32() const
{
	return BinaryReaderDetail::generic_read<UInt32>(*stream_);
}

Int64 BinaryReader::read_s64() const
{
	return BinaryReaderDetail::generic_read<Int64>(*stream_);
}

UInt64 BinaryReader::read_u64() const
{
	return BinaryReaderDetail::generic_read<UInt64>(*stream_);
}

float BinaryReader::read_b32() const
{
	return BinaryReaderDetail::generic_read<float>(*stream_);
}

double BinaryReader::read_b64() const
{
	return BinaryReaderDetail::generic_read<double>(*stream_);
}

} // namespace bstone
