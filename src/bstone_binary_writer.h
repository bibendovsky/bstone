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


#ifndef BSTONE_BINARY_WRITER_INCLUDED
#define BSTONE_BINARY_WRITER_INCLUDED


#include "bstone_stream.h"


namespace bstone
{


class BinaryWriter final
{
public:
	BinaryWriter(
		Stream* stream = nullptr);

	BinaryWriter(
		BinaryWriter&& rhs);

	BinaryWriter(
		const BinaryWriter& that) = delete;

	BinaryWriter& operator=(
		const BinaryWriter& that) = delete;


	bool open(
		Stream* stream);

	// Closes the writer but stream.
	void close();

	bool is_open() const;

	// Writes a signed 8-bit integer value.
	bool write_s8(
		const std::int8_t value);

	// Writes an unsigned 8-bit integer value.
	bool write_u8(
		const std::uint8_t value);

	// Writes a signed 16-bit integer value.
	bool write_s16(
		const std::int16_t value);

	// Writes an unsigned 16-bit integer value.
	bool write_u16(
		const std::uint16_t value);

	// Writes a signed 32-bit integer value.
	bool write_s32(
		const std::int32_t value);

	// Writes an unsigned 32-bit integer value.
	bool write_u32(
		const std::uint32_t value);

	// Writes a signed 64-bit integer value.
	bool write_s64(
		const std::int64_t value);

	// Writes an unsigned 64-bit integer value.
	bool write_u64(
		const std::uint64_t value);

	// Writes a 32-bit float-point value.
	bool write_r32(
		const float value);

	// Writes a 64-bit float-point value.
	bool write_r64(
		const double value);

	bool write(
		const void* buffer,
		const int count);

	// Writes a string prepended with signed 32-bit (little-endian) length.
	bool write(
		const std::string& string);

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

	template<typename T>
	bool write(
		const T& value)
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->write(&value, sizeof(T));
	}

	template<typename T, std::size_t N>
	bool write(
		const T(&value)[N])
	{
		if (!is_open())
		{
			return false;
		}

		return stream_->write(value, N * sizeof(T));
	}


private:
	Stream* stream_;
}; // BinaryWriter


} // bstone


#endif // BSTONE_BINARY_WRITER_INCLUDED
