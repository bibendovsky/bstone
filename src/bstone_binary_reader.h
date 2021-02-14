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


#ifndef BSTONE_BINARY_READER_INCLUDED
#define BSTONE_BINARY_READER_INCLUDED


#include "bstone_stream.h"


namespace bstone
{


class BinaryReader
{
public:
	BinaryReader(
		Stream* stream = nullptr);

	BinaryReader(
		BinaryReader&& rhs);

	BinaryReader(
		const BinaryReader& rhs) = delete;

	BinaryReader& operator=(
		const BinaryReader& rhs) = delete;


	bool open(
		Stream* stream);

	// Closes the reader but stream.
	void close();

	bool is_open() const;

	// Reads a signed 8-bit integer value.
	std::int8_t read_s8();

	// Reads an unsigned 8-bit integer value.
	std::uint8_t read_u8();

	// Reads a signed 16-bit integer value.
	std::int16_t read_s16();

	// Reads an unsigned 16-bit integer value.
	std::uint16_t read_u16();

	// Reads a signed 32-bit integer value.
	std::int32_t read_s32();

	// Reads an unsigned 32-bit integer value.
	std::uint32_t read_u32();

	// Reads a signed 64-bit integer value.
	std::int64_t read_s64();

	// Reads an unsigned 64-bit integer value.
	std::uint64_t read_u64();

	// Reads a 32-bit float-point value.
	float read_r32();

	// Reads a 64-bit float-point value.
	double read_r64();

	// Reads a string prepended with signed 32-bit (little-endian) length.
	// Returns empty string on error or when max length reached.
	std::string read_string(
		const int max_length = -1);

	bool read(
		void* buffer,
		const int count);


	template<typename T>
	bool read(
		T& value)
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->read(&value, sizeof(T)) == sizeof(T);
	}

	template<typename T, std::size_t N>
	bool read(
		T(&value)[N])
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->read(value, N * sizeof(T)) == N * sizeof(T);
	}

	// Skips a number of octets forward if count is positive or
	// backward otherwise.
	// Returns false on error.
	bool skip(
		const int count);

	// Returns a current position.
	std::int64_t get_position() const;

	// Sets a current position to a specified one.
	bool set_position(
		const std::int64_t position);

private:
	Stream* stream_;


	template<typename T>
	T read()
	{
		if (!is_open())
		{
			return 0;
		}

		T value;

		if (stream_->read(&value, sizeof(T)) != sizeof(T))
		{
			return 0;
		}

		return value;
	}
}; // BinaryReader


} // bstone


#endif // BSTONE_BINARY_READER_INCLUDED
