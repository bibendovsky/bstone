/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
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
		auto value = T{};
		stream.read_exactly(&value, static_cast<std::intptr_t>(sizeof(T)));
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

std::int8_t BinaryReader::read_s8() const
{
	return BinaryReaderDetail::generic_read<std::int8_t>(*stream_);
}

std::uint8_t BinaryReader::read_u8() const
{
	return BinaryReaderDetail::generic_read<std::uint8_t>(*stream_);
}

std::int16_t BinaryReader::read_s16() const
{
	return BinaryReaderDetail::generic_read<std::int16_t>(*stream_);
}

std::uint16_t BinaryReader::read_u16() const
{
	return BinaryReaderDetail::generic_read<std::uint16_t>(*stream_);
}

std::int32_t BinaryReader::read_s32() const
{
	return BinaryReaderDetail::generic_read<std::int32_t>(*stream_);
}

std::uint32_t BinaryReader::read_u32() const
{
	return BinaryReaderDetail::generic_read<std::uint32_t>(*stream_);
}

std::int64_t BinaryReader::read_s64() const
{
	return BinaryReaderDetail::generic_read<std::int64_t>(*stream_);
}

std::uint64_t BinaryReader::read_u64() const
{
	return BinaryReaderDetail::generic_read<std::uint64_t>(*stream_);
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
