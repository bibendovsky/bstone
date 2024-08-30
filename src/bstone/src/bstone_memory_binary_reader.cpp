/*
BStone: Unofficial source port of Blake Stone: Aliens of Gold and Blake Stone: Planet Strike
Copyright (c) 2013-2024 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
SPDX-License-Identifier: MIT
*/


//
// A binary reader for a block of memory.
//


#include "bstone_memory_binary_reader.h"

#include <algorithm>
#include <memory>

#include "bstone_endian.h"


namespace bstone
{


MemoryBinaryReader::MemoryBinaryReader()
	:
	data_{},
	data_size_{},
	data_offset_{}
{
}

MemoryBinaryReader::MemoryBinaryReader(
	const void* data,
	const std::int64_t data_size)
{
	static_cast<void>(open(data, data_size));
}

bool MemoryBinaryReader::is_initialized() const
{
	return data_ != nullptr;
}

bool MemoryBinaryReader::open(
	const void* data,
	const std::int64_t data_size)
{
	close();

	if (!data)
	{
		return false;
	}

	if (data_size <= 0)
	{
		return false;
	}

	data_ = static_cast<const std::uint8_t*>(data);
	data_size_ = data_size;
	data_offset_ = 0;

	return true;
}

void MemoryBinaryReader::close()
{
	data_ = nullptr;
	data_size_ = 0;
	data_offset_ = 0;
}

std::int8_t MemoryBinaryReader::read_s8()
{
	return read<std::int8_t>();
}

std::uint8_t MemoryBinaryReader::read_u8()
{
	return read<std::uint8_t>();
}

std::int16_t MemoryBinaryReader::read_s16()
{
	return read<std::int16_t>();
}

std::uint16_t MemoryBinaryReader::read_u16()
{
	return read<std::uint16_t>();
}

std::int32_t MemoryBinaryReader::read_s32()
{
	return read<std::int32_t>();
}

std::uint32_t MemoryBinaryReader::read_u32()
{
	return read<std::uint32_t>();
}

std::int64_t MemoryBinaryReader::read_s64()
{
	return read<std::int64_t>();
}

std::uint64_t MemoryBinaryReader::read_u64()
{
	return read<std::uint64_t>();
}

float MemoryBinaryReader::read_r32()
{
	return read<float>();
}

double MemoryBinaryReader::read_r64()
{
	return read<double>();
}

std::string MemoryBinaryReader::read_string()
{
	auto length = bstone::endian::to_little(read_s32());

	if (length == 0)
	{
		return {};
	}

	std::string string(length, '\0');

	if (length > 0)
	{
		if (!read(&string[0], length))
		{
			return {};
		}
	}

	return string;
}

bool MemoryBinaryReader::read(
	void* buffer,
	const int count)
{
	if (!buffer)
	{
		return false;
	}

	if (count <= 0)
	{
		return true;
	}

	if (!is_initialized())
	{
		return false;
	}

	if (data_offset_ < 0)
	{
		return false;
	}

	if ((data_offset_ + count) >= data_size_)
	{
		return false;
	}

	std::copy_n(&data_[data_offset_], count, static_cast<std::uint8_t*>(buffer));

	data_offset_ += count;

	return true;
}

bool MemoryBinaryReader::skip(
	const std::int64_t count)
{
	if (!is_initialized())
	{
		return false;
	}

	auto new_offset = data_offset_ + count;

	if (new_offset < 0)
	{
		return false;
	}

	data_offset_ = new_offset;

	return true;
}

std::int64_t MemoryBinaryReader::get_position() const
{
	return data_offset_;
}

bool MemoryBinaryReader::set_position(
	const std::int64_t position)
{
	if (!is_initialized())
	{
		return false;
	}

	if (position < 0)
	{
		return false;
	}

	data_offset_ = position;

	return true;
}


} // bstone
