/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <utility>
#include "bstone_binary_writer.h"
#include "bstone_endian.h"

namespace bstone
{

BinaryWriter::BinaryWriter(Stream* stream)
{
	static_cast<void>(open(stream));
}

BinaryWriter::BinaryWriter(BinaryWriter&& rhs)
{
	std::swap(stream_, rhs.stream_);
}

bool BinaryWriter::open(Stream* stream)
{
	close();

	if (!stream)
	{
		return false;
	}

	if (!stream->is_writable())
	{
		return false;
	}

	stream_ = stream;
	return true;
}

void BinaryWriter::close()
{
	stream_ = nullptr;
}

bool BinaryWriter::is_open() const
{
	return stream_ != nullptr;
}

bool BinaryWriter::write_s8(std::int8_t value)
{
	return write(value);
}

bool BinaryWriter::write_u8(std::uint8_t value)
{
	return write(value);
}

bool BinaryWriter::write_s16(std::int16_t value)
{
	return write(value);
}

bool BinaryWriter::write_u16(std::uint16_t value)
{
	return write(value);
}

bool BinaryWriter::write_s32(std::int32_t value)
{
	return write(value);
}

bool BinaryWriter::write_u32(std::uint32_t value)
{
	return write(value);
}

bool BinaryWriter::write_s64(std::int64_t value)
{
	return write(value);
}

bool BinaryWriter::write_u64(std::uint64_t value)
{
	return write(value);
}

bool BinaryWriter::write_r32(float value)
{
	return write(value);
}

bool BinaryWriter::write_r64(double value)
{
	return write(value);
}

bool BinaryWriter::write(const void* buffer, int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->write(buffer, count);
}

bool BinaryWriter::write(const std::string& string)
{
	const auto length = static_cast<std::int32_t>(string.length());
	auto is_succeed = true;
	is_succeed &= write_s32(bstone::endian::to_little(length));
	is_succeed &= write(string.c_str(), length);
	return is_succeed;
}

bool BinaryWriter::skip(int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->skip(count) >= 0;
}

int BinaryWriter::get_position() const
{
	if (!is_open())
	{
		return 0;
	}

	return stream_->get_position();
}

bool BinaryWriter::set_position(int position)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->set_position(position);
}

} // bstone
