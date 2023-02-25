/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/

#include <utility>
#include "bstone_binary_reader.h"
#include "bstone_endian.h"

namespace bstone
{

BinaryReader::BinaryReader(Stream* stream)
{
	static_cast<void>(open(stream));
}

BinaryReader::BinaryReader(BinaryReader&& rhs)
{
	std::swap(stream_, rhs.stream_);
}

bool BinaryReader::open(Stream* stream)
{
	close();

	if (!stream)
	{
		return false;
	}

	if (!stream->is_readable())
	{
		return false;
	}

	stream_ = stream;
	return true;
}

void BinaryReader::close()
{
	stream_ = nullptr;
}

bool BinaryReader::is_open() const
{
	return stream_ != nullptr;
}

std::int8_t BinaryReader::read_s8()
{
	return read<std::int8_t>();
}

std::uint8_t BinaryReader::read_u8()
{
	return read<std::uint8_t>();
}

std::int16_t BinaryReader::read_s16()
{
	return read<std::int16_t>();
}

std::uint16_t BinaryReader::read_u16()
{
	return read<std::uint16_t>();
}

std::int32_t BinaryReader::read_s32()
{
	return read<std::int32_t>();
}

std::uint32_t BinaryReader::read_u32()
{
	return read<std::uint32_t>();
}

std::int64_t BinaryReader::read_s64()
{
	return read<std::int64_t>();
}

std::uint64_t BinaryReader::read_u64()
{
	return read<std::uint64_t>();
}

float BinaryReader::read_r32()
{
	return read<float>();
}

double BinaryReader::read_r64()
{
	return read<double>();
}

std::string BinaryReader::read_string(int max_length)
{
	const auto length = bstone::endian::to_little(read_s32());

	if (max_length >= 0 && length > max_length)
	{
		return std::string{};
	}

	std::string string(length, '\0');

	if (length > 0)
	{
		if (!read(&string[0], length))
		{
			string.clear();
		}
	}

	return string;
}

bool BinaryReader::read(void* buffer, int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->read(buffer, count) == count;
}

bool BinaryReader::skip(int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->skip(count) >= 0;
}

int BinaryReader::get_position() const
{
	if (!is_open())
	{
		return 0;
	}

	return stream_->get_position();
}

bool BinaryReader::set_position(int position)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->set_position(position);
}

} // bstone
