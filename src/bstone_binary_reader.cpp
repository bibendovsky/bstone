/*
BStone: A Source port of
Blake Stone: Aliens of Gold and Blake Stone: Planet Strike

Copyright (c) 1992-2013 Apogee Entertainment, LLC
Copyright (c) 2013-2021 Boris I. Bendovsky (bibendovsky@hotmail.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the
Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "bstone_binary_reader.h"

#include "bstone_endian.h"


namespace bstone
{


BinaryReader::BinaryReader(
	Stream* stream)
	:
	stream_{}
{
	static_cast<void>(open(stream));
}

BinaryReader::BinaryReader(
	BinaryReader&& rhs)
	:
	stream_{std::move(rhs.stream_)}
{
	rhs.stream_ = nullptr;
}

bool BinaryReader::open(
	Stream* stream)
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

std::string BinaryReader::read_string(
	const int max_length)
{
	const auto length = bstone::Endian::little(read_s32());

	if (max_length >= 0 && length > max_length)
	{
		return {};
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

bool BinaryReader::read(
	void* buffer,
	const int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->read(buffer, count) == count;
}

bool BinaryReader::skip(
	const int count)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->skip(count) >= 0;
}

std::int64_t BinaryReader::get_position() const
{
	if (!is_open())
	{
		return 0;
	}

	return stream_->get_position();
}

bool BinaryReader::set_position(
	std::int64_t position)
{
	if (!is_open())
	{
		return false;
	}

	return stream_->set_position(position);
}


} // bstone
