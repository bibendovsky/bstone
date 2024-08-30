/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

// Writes primitive data types as binary values.

#include <limits>

#include "bstone_binary_writer.h"

static_assert(std::numeric_limits<float>::is_iec559 && sizeof(float) == 4, "Unsupported `float` type.");
static_assert(std::numeric_limits<double>::is_iec559 && sizeof(double) == 8, "Unsupported `double` type.");

namespace bstone {

namespace {

struct BinaryWriterDetail
{
	template<typename T>
	static void generic_write(Stream& stream, T value)
	{
		stream.write_exactly(&value, static_cast<std::intptr_t>(sizeof(T)));
	}
};

} // namespace

// ==========================================================================

BinaryWriter::BinaryWriter(Stream& stream)
	:
	stream_{&stream}
{}

Stream& BinaryWriter::get_stream() const
{
	return *stream_;
}

void BinaryWriter::write_s8(std::int8_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_u8(std::uint8_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_s16(std::int16_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_u16(std::uint16_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_s32(std::int32_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_u32(std::uint32_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_s64(std::int64_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_u64(std::uint64_t value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_b32(float value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

void BinaryWriter::write_b64(double value) const
{
	BinaryWriterDetail::generic_write(*stream_, value);
}

} // namespace bstone
